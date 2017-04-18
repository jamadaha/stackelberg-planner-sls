
#include "best_first_search.h"
#include "globals.h"
#include "open_list_factory.h"

#include "../state_registry.h"
#include "../successor_generator.h"

#include "../global_state.h"
#include "../global_operator.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "../globals.h"
#include "../timer.h"

#include <limits>
#include <string>
#include <iostream>
#include <cstdio>

#define WORST_ATTACKER_REWARD std::numeric_limits<int>::max()

namespace second_order_search
{

SORBestFirstSearch::SORBestFirstSearch(const Options &opts)
    : SearchEngine(opts),
      m_open_list(parse_open_list<StateID>(opts.get_enum("open_list"))),
      m_inner_search(opts.get<SearchEngine*>("inner_search")),
      m_pruning_method(opts.contains("pruning_method") ?
                       opts.get<SuccessorPruningMethod *>("pruning_method") : NULL)
{
    m_g_limit = std::numeric_limits<int>::max();

    m_stat_open = 0;
    m_stat_expanded = 0;
    m_stat_generated = 1;
    m_stat_pruned_successors = 0;

    m_stat_last_printed_states = 0;
    m_stat_last_printed_pareto = 0;
}

void SORBestFirstSearch::initialize()
{
    preprocess_second_order_task();
    std::cout << "Initializing 2OT best-first search ..."
              << std::endl;
    if (m_pruning_method != NULL) {
        m_pruning_method->initialize();
    }
    GlobalState init = g_outer_initial_state();
    SearchNode node = m_search_space[init];
    node.open_initial();
    m_open_list->push(node.get_info(), init.get_id());
}

void SORBestFirstSearch::insert_into_pareto_frontier(const SearchNode &node)
{
    typename ParetoFrontier::iterator it
        = m_pareto_frontier.lower_bound(node.get_reward());

    if ((it != m_pareto_frontier.end() && it->first == node.get_reward()
            && it->second.first < node.get_g())
            || (it != m_pareto_frontier.begin()
                && ((--it)++)->second.first <= node.get_g())) {
        return;
    }

    if (it != m_pareto_frontier.end() && it->first == node.get_reward()) {
        if (it->second.first > node.get_g()) {
            it->second.first = node.get_g();
            it->second.second.clear();
        }
        it->second.second.push_back(node.get_state_id());
    } else {
        std::pair<int, std::vector<StateID> > &entry =
            m_pareto_frontier[node.get_reward()];
        entry.first = node.get_g();
        entry.second.push_back(node.get_state_id());
        it = m_pareto_frontier.lower_bound(node.get_reward());
    }

    assert(it != m_pareto_frontier.end());

    it++;
    while (it != m_pareto_frontier.end() && it->second.first >= node.get_g()) {
        it = m_pareto_frontier.erase(it);
    }
}

SearchStatus SORBestFirstSearch::step()
{
    if (m_open_list->empty()) {
        solution_found = true;
        return SOLVED;
    }

    GlobalState state = g_outer_state_registry->lookup_state(m_open_list->pop());
    SearchNode node = m_search_space[state];
    if (node.is_closed()) {
        return IN_PROGRESS;
    }

    node.close();
    m_stat_open--;

    // m_g_limit might have been updated since state was added to open
    if (node.get_g() > m_g_limit) {
        return IN_PROGRESS;
    }

    m_stat_expanded++;

    // NOTE in mitit the parent attack plan is not applicable in the child state
    // if S3 pruning is enabled; thus the check for applicable attack plans is
    // not implemented here

    // compute induced inner task
    g_operators.clear();
    g_outer_inner_successor_generator->generate_applicable_ops(state,
            m_applicable_operators);
    for (const GlobalOperator *op : m_applicable_operators) {
        g_operators.push_back(*op);
    }
    m_applicable_operators.clear();
    // NOTE for mitit g_successor_generator is not required, and thus will not
    // be constructed

    // solve induced inner task:
    streambuf *old = cout.rdbuf(); // <-- save
    stringstream ss;
    cout.rdbuf(ss.rdbuf());        // <-- redirect
    m_inner_search->reset();
    m_inner_search->search();
    if (!m_inner_search->found_solution()) {
        cout.rdbuf(old);   			// <-- restore
        node.set_reward(WORST_ATTACKER_REWARD);
        insert_into_pareto_frontier(node);
        if (node.get_g() < m_g_limit) {
            m_g_limit = node.get_g();
        }
        // everything from here on will be dominated
        return IN_PROGRESS;
    } else {
        m_inner_search->save_plan_if_necessary();
        cout.rdbuf(old);   			// <-- restore
    }

    node.set_reward(m_inner_search->calculate_plan_cost());
    insert_into_pareto_frontier(node);

    // generate outer successor states
    g_outer_successor_generator->generate_applicable_ops(state,
            m_applicable_operators);
    m_stat_pruned_successors += m_applicable_operators.size();
    if (m_pruning_method != NULL) {
        m_pruning_method->prune_successors(state, g_plan, m_applicable_operators);
    }
    m_stat_pruned_successors -= m_applicable_operators.size();
    m_stat_generated += m_applicable_operators.size();
    for (unsigned i = 0; i < m_applicable_operators.size(); i++) {
        int succ_g = node.get_g() + get_adjusted_cost(*m_applicable_operators[i]);
        if (succ_g > m_g_limit) {
            continue;
        }
        GlobalState succ = g_outer_state_registry->get_successor_state(state,
                           *m_applicable_operators[i]);
        SearchNode succ_node = m_search_space[succ];
        succ_node.set_reward(node.get_reward());
        if (succ_node.is_new()
                || (succ_node.is_open() && succ_g < succ_node.get_g())) {
            if (succ_node.is_new()) {
                m_stat_open++;
            }
            succ_node.open(state.get_id(), m_applicable_operators[i], succ_g);
            m_open_list->push(succ_node.get_info(), succ.get_id());
        }
#ifdef COMPUTE_COMPLETE_PARETO_FRONTIER
        else if (succ_node.get_g() == succ_g) {
            succ_node.add_parent(m_applicable_operators[i], state.get_id());
        }
#endif
    }
    m_applicable_operators.clear();

    print_statistic_line();

    return IN_PROGRESS;
}

void SORBestFirstSearch::save_plan_if_necessary()
{
    std::cout << "(2OT) Pareto frontier contains " << m_pareto_frontier.size() <<
              " groups" << std::endl;
    std::cout << "---begin-pareto-frontier---" << std::endl;
    unsigned num = 1;
    size_t num_states = 0;
    for (typename ParetoFrontier::reverse_iterator it = m_pareto_frontier.rbegin();
            it != m_pareto_frontier.rend();
            it++) {
        std::cout << "    ---group-" << num << "--- {"
                  << "reward: " << it->first
                  << ", cost: " << it->second.first
                  << "}" << std::endl;
        size_t counter = 1;
        for (unsigned i = 0; i < it->second.second.size(); i++) {
            m_search_space.print_backtrace(it->second.second[i], counter);
            num_states++;
        }
        num++;
    }
    std::cout << "---end-pareto-frontier---" << std::endl;
    std::cout << "(2OT) state(s) in Pareto frontier: " << num_states << std::endl;
    std::cout << "(2OT) registered state(s): " << g_outer_state_registry->size() <<
              std::endl;
    std::cout << "(2OT) expanded state(s): " << m_stat_expanded << std::endl;
    std::cout << "(2OT) generated state(s): " << m_stat_generated << std::endl;
    std::cout << "(2OT) pruned successors: " << m_stat_pruned_successors <<
              std::endl;
}

void SORBestFirstSearch::print_statistic_line()
{
    if (m_stat_last_printed_states * 2 <= m_stat_expanded
            || m_stat_last_printed_pareto != m_pareto_frontier.size()) {
        m_stat_last_printed_states = m_stat_expanded;
        m_stat_last_printed_pareto = m_pareto_frontier.size();
        force_print_statistic_line();
    }
}

void SORBestFirstSearch::force_print_statistic_line() const
{
    assert(!m_pareto_frontier.empty());
    printf("[P={(%d, %d)..(%d, %d)} (%zu), expanded=%zu, open=%zu, pruned=%zu, t=%.3fs]\n",
           m_pareto_frontier.begin()->second.first,
           m_pareto_frontier.begin()->first,
           m_pareto_frontier.rbegin()->second.first,
           m_pareto_frontier.rbegin()->first,
           m_pareto_frontier.size(),
           m_stat_expanded,
           m_stat_open,
           m_stat_pruned_successors,
           g_timer());
}

void SORBestFirstSearch::add_options_to_parser(OptionParser &parser)
{
    parser.add_option<SearchEngine *>("inner_search", "", "delrax");

    std::vector<std::string> open_lists;
    get_open_list_options(open_lists);
    parser.add_enum_option("open_list", open_lists, "", open_lists.front());

    parser.add_option<SuccessorPruningMethod *>("pruning_method", "", "",
            OptionFlags(false));

    SearchEngine::add_options_to_parser(parser);
}

}

static SearchEngine *_parse(OptionParser &parser)
{
    second_order_search::SORBestFirstSearch::add_options_to_parser(parser);
    Options opts = parser.parse();
    if (!parser.dry_run()) {
        return new second_order_search::SORBestFirstSearch(opts);
    }
    return NULL;
}

static Plugin<SearchEngine> _plugin("2otbfs", _parse);
