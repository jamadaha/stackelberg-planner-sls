
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

#include "../int_packer.h"

#include "../delrax_search.h"

#include "../utilities.h"

#include <limits>
#include <string>
#include <iostream>
#include <cstdio>

#include <sstream>
#include <fstream>

#ifndef NDEBUG
// #define VERBOSE_DEBUGGING
#endif

namespace second_order_search
{

BestFirstSearch::BestFirstSearch(const Options &opts)
    : SecondOrderTaskSearch(opts),
      c_silent(opts.get<bool>("silent")),
      c_precompute_max_reward(opts.get<bool>("precompute_max_reward")),
      c_lazy_reward_computation(opts.get<bool>("lazy")),
      c_sleep_set(opts.get<bool>("sleep_set")),
      m_open_list(parse_open_list<StateID>(opts.get_enum("open_list"))),
      m_pruning_method(opts.contains("pruning_method") ?
                       opts.get<SuccessorPruningMethod *>("pruning_method") : NULL)
{
    if (!c_lazy_reward_computation && !c_incremental_rpg) {
        std::cerr << "!incremental_rpg requires lazy=true"  << std::endl;
        exit_with(EXIT_CRITICAL_ERROR);
    }
    m_g_limit = std::numeric_limits<int>::max();
    m_max_reward = MAX_REWARD;

    m_stat_open = 1;

    m_stat_expanded = 0;
    m_stat_generated = 1;
    m_stat_pruned_successors = 0;
    m_stat_evaluated = 0;

    m_stat_last_printed_states = 0;
    m_stat_last_printed_pareto = 0;

    m_stat_inner_searches = 0;
    m_stat_time_inner_search.reset();
    m_stat_time_inner_search.stop();

    m_stat_current_g = -1;
    m_stat_expanded_last_f_layer = 0;
    m_stat_last_g = -1;
}

void BestFirstSearch::initialize()
{
    SecondOrderTaskSearch::initialize();
    std::cout << "Initializing 2OT best-first search";
    if (c_lazy_reward_computation) {
        std::cout << " with lazy reward-computation";
    }
    std::cout << " ..." << std::endl;
    if (m_pruning_method != NULL) {
        m_pruning_method->initialize();
    }

    // computing worst-attacker reward
    // *NOTE* this computation is correct only in the mitit case
    if (c_precompute_max_reward) {
        m_max_reward = compute_max_reward();
    }

    GlobalState init = g_outer_initial_state();
    SearchNode node = m_search_space[init];
    node.open_initial();
    if (c_incremental_rpg) {
        node.set_reward(get_initial_state_reward(node.get_counter()));
    } else {
        node.set_reward(run_inner_search(init));
    }

    m_open_list->push(node.get_info(), init.get_id());

    if (c_sleep_set) {
        std::vector<int> ranges(g_outer_operators.size(), 2);
        m_sleep_packer = new IntPacker(ranges);
        node.get_sleep() = new IntPacker::Bin[m_sleep_packer->get_num_bins()];
        for (size_t i = 0; i < g_outer_operators.size(); i++) {
            m_sleep_packer->set(node.get_sleep(), i, 0);
        }
        std::cout << "Bytes per sleep_set: "
                  << (m_sleep_packer->get_num_bins() * sizeof(IntPacker::Bin))
                  << std::endl;
    }

    std::cout << "Maximal possible reward is: " << m_max_reward << std::endl;
}

IntPacker::Bin *BestFirstSearch::create_sleep_set_copy(IntPacker::Bin *x)
{
    IntPacker::Bin *sleep = new IntPacker::Bin[m_sleep_packer->get_num_bins()];
    for (int i = 0; i < m_sleep_packer->get_num_bins(); i++) {
        sleep[i] = x[i];
    }
    return sleep;
}

void BestFirstSearch::insert_into_pareto_frontier(const SearchNode &node)
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

void BestFirstSearch::compute_and_set_reward(const SearchNode &parent,
        const GlobalOperator &op,
        SearchNode &node)
{
    m_stat_inner_searches++;
    m_stat_time_inner_search.resume();

    if (c_incremental_rpg) {
        node.set_reward(parent.get_reward()
                        - compute_reward_difference(parent.get_counter(),
                                op,
                                node.get_counter()));

        // g_outer_inner_successor_generator->generate_applicable_ops(
        //     g_outer_state_registry->lookup_state(node.get_state_id()),
        //     m_available_operators);
        // for (const auto &x : m_available_operators) {
        //     assert(!std::count(m_outer_to_inner_operator[op.get_op_id()].begin(),
        //                        m_outer_to_inner_operator[op.get_op_id()].end(),
        //                        x->get_op_id() + g_variable_domain.size()));
        // }
        // m_available_operators.clear();
        //
        // std::vector<int> test;
        // rgraph_exploration(g_outer_state_registry->lookup_state(node.get_state_id()),
        //                    test);
        // std::cout << get_reward(test)
        //           << ":" << get_reward(node.get_counter())
        //           << ":" << node.get_reward()
        //           << "::" << parent.get_reward()
        //           << ":" << get_reward(parent.get_counter())
        //           << std::endl;
        // std::cout << op.get_name() << std::endl;
        // for (unsigned i = 0; i < test.size(); i++) {
        //     if (test[i] != get_counter_packer()->get(node.get_counter(), i)) {
        //         std::cout << "DIFF [";
        //         if (i < g_variable_domain.size()) {
        //             std::cout << g_fact_names[i][0];
        //         } else {
        //             std::cout << g_inner_operators[i - g_variable_domain.size()].get_name();
        //         }
        //         std::cout << "] -> " << test[i] << " vs " << get_counter_packer()->get(
        //                       node.get_counter(), i) << std::endl;
        //     }
        // }
        // assert(container_equals(test,
        //                         RandomAccessBin(get_counter_packer(), node.get_counter()),
        //                         test.size()));
        // assert(get_reward(test) == get_reward(node.get_counter()));
        // assert(get_reward(test) == node.get_reward());
    } else {
        // node.set_reward()
        node.set_reward(run_inner_search(g_outer_state_registry->lookup_state(
                                             node.get_state_id())));
    }

    m_stat_time_inner_search.stop();
}

void BestFirstSearch::set_inner_plan(SearchNode &node)
{
    m_stat_time_inner_search.resume();
    if (c_incremental_rpg) {
        extract_inner_plan(node.get_counter(), g_plan);
    } else {
        assert(m_inner_search->found_solution());
        m_inner_search->save_plan_if_necessary();
    }
    m_stat_time_inner_search.stop();
}

int BestFirstSearch::compute_max_reward()
{
    int res;
    IntPacker temp_packer(g_outer_variable_domain);
    PackedStateBin *buffer = new PackedStateBin[temp_packer.get_num_bins()];
    for (const auto &op : g_outer_operators) {
        for (const auto &e : op.get_effects()) {
            temp_packer.set(buffer, e.var, e.val);
        }
    }
    GlobalState best_state(buffer, NULL, StateID(std::numeric_limits<int>::max()),
                           &temp_packer);
    if (c_incremental_rpg) {
        std::vector<int> counter;
        rgraph_exploration(best_state, counter);
        res = get_reward(counter);
    } else {
        res = run_inner_search(best_state);
    }
    delete[] buffer;
    return res;
}

bool op_ptr_name_comp(const GlobalOperator *op1, const GlobalOperator *op2)
{
    return op1->get_name() < op2->get_name();
}

string BestFirstSearch::fix_state_to_string(const GlobalState &state)
{
    string res = "";
    for (size_t i = 0; i < g_outer_variable_domain.size(); i++) {
        res += to_string(state[i]);
    }
    return res;
}

string BestFirstSearch::ops_to_string(vector<const GlobalOperator *> &ops)
{
    sort(ops.begin(), ops.end(), op_ptr_name_comp);
    string res = "";
    for (size_t i = 0; i < ops.size(); i++) {
        if (i > 0) {
            res += " ";
        }
        res += ops[i]->get_name();
    }
    return res;
}

SearchStatus BestFirstSearch::step()
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
        solution_found = true;
        return SOLVED;
    }

    m_stat_current_g = node.get_g();

    print_statistic_line();

    m_stat_expanded++;

    // NOTE in mitit the parent attack plan is not applicable in the child state
    // if S3 pruning is enabled; thus the check for applicable attack plans is
    // not implemented here
    // node.set_reward(compute_reward(state));
    if (c_lazy_reward_computation
            && node.get_parent_state_id() != StateID::no_state) {
        compute_and_set_reward(m_search_space[node.get_parent_state_id()],
                               *node.get_parent_operator(),
                               node);
    }
    insert_into_pareto_frontier(node);

    // std::cout << "---STATE#" << state.get_id().hash() << "--- (" << node.get_g() <<
    //           ", " << node.get_reward() << ")"
    //           << std::endl;
    if (node.get_reward() == m_max_reward || node.get_g() == m_g_limit) {
        // std::cout << "<none>" << std::endl;
        if (node.get_g() < m_g_limit) {
            m_g_limit = node.get_g();
        }
        // everything from here on will be dominated
        return IN_PROGRESS;
    }

    // (dynamic_cast<delrax_search::DelRaxSearch *>
    //  (m_inner_search))->dump_achieved_goal_facts();
    // std::cout << "---end---" << std::endl;

    // generate outer successor states
    g_outer_successor_generator->generate_applicable_ops(state,
            m_applicable_operators);
    m_stat_pruned_successors += m_applicable_operators.size();
    if (m_pruning_method != NULL) {
        set_inner_plan(node);
        // std::cout << "---plan#" << state.get_id().hash() << "---" << std::endl;
        // for (unsigned i = 0; i < g_plan.size(); i++) {
        //     std::cout << g_plan[i]->get_name() << std::endl;
        // }
        // std::cout << std::endl;
        m_pruning_method->prune_successors(state, g_plan, m_applicable_operators);

        /*vector<const GlobalOperator *> ops(g_plan.size());
        for (size_t op_no = 0; op_no < g_plan.size(); op_no++) {
        	ops[op_no] = g_plan[op_no];
        }
        cerr << fix_state_to_string(state) << ": " << ops_to_string(ops) << endl;*/

        g_plan.clear();
    }

    cerr << fix_state_to_string(state) << ": " << ops_to_string(
             m_applicable_operators) << endl;

    m_stat_pruned_successors -= m_applicable_operators.size();
    m_stat_generated += m_applicable_operators.size();
    for (unsigned i = 0; i < m_applicable_operators.size(); i++) {
        if (c_sleep_set) {
            if (m_sleep_packer->get(node.get_sleep(),
                                    m_applicable_operators[i]->get_op_id())) {
                continue;
            }
            m_sleep_packer->set(node.get_sleep(),
                                m_applicable_operators[i]->get_op_id(),
                                1);
        }
        int succ_g = node.get_g() + get_adjusted_cost(*m_applicable_operators[i]);
        if (succ_g > m_g_limit) {
            continue;
        }
        GlobalState succ = g_outer_state_registry->get_successor_state(state,
                           *m_applicable_operators[i]);
        SearchNode succ_node = m_search_space[succ];
        // *NOTE* only correct for monotone 2OR tasks

        if (succ_node.is_new()) {
            m_stat_open++;
            // TODO heuristic computation goes here
            m_stat_evaluated++;
            if (!c_lazy_reward_computation) {
                compute_and_set_reward(node, *m_applicable_operators[i], succ_node);
            } else {
                succ_node.set_reward(node.get_reward());
            }
            if (c_sleep_set) {
                succ_node.get_sleep() = create_sleep_set_copy(node.get_sleep());
            }
        }

        if (succ_node.is_new()
                || (succ_node.is_open() && succ_g < succ_node.get_g())) {
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

    if (!c_lazy_reward_computation) {
        delete (node.get_counter());
        node.get_counter() = NULL;
        assert(node.get_counter() == NULL);
    }

    if (c_sleep_set) {
        delete (node.get_sleep());
        node.get_sleep() = NULL;
    }

    return IN_PROGRESS;
}

void BestFirstSearch::save_plan_if_necessary()
{
    std::cout << "(2OT) Pareto frontier consists of " << m_pareto_frontier.size() <<
              " groups" << std::endl;
    std::ostringstream json;
    json << "[";
    std::vector<std::vector<const GlobalOperator *> > paths;
    size_t num_states = 0;


    std::streambuf *old = NULL;
    if (c_silent) {
        old = std::cout.rdbuf(); // <-- save
        std::stringstream ss;
        std::cout.rdbuf(ss.rdbuf());        // <-- redirect
    }

    std::cout << "---begin-pareto-frontier---" << std::endl;
    unsigned num = 1;
    for (typename ParetoFrontier::reverse_iterator it = m_pareto_frontier.rbegin();
            it != m_pareto_frontier.rend();
            it++) {

        if (it != m_pareto_frontier.rbegin()) {
            json << ",\n";
        }
        json << "{"
             << "\"reward\": " << it->first
             << ", \"cost\": " << it->second.first
             << ", \"sequences\": [";

        std::cout << "    ---group-" << num << "--- {"
                  << "reward: " << it->first
                  << ", cost: " << it->second.first
                  << "}" << std::endl;

        size_t counter = 1;
        for (unsigned i = 0; i < it->second.second.size(); i++) {
#ifdef VERBOSE_DEBUGGING
            std::cout << "        ---begin-state-" << i << "--- [" <<
                      it->second.second[i].hash() << "]" << std::endl;
            GlobalState state = g_outer_state_registry->lookup_state(it->second.second[i]);
            for (unsigned var = 0; var < g_outer_variable_domain.size(); var++) {
                std::cout << "        " << g_outer_fact_names[var][state[var]] << std::endl;
            }
            std::cout << "        ---end-state---" << std::endl;
#endif

            m_search_space.backtrace(it->second.second[i], paths);
            for (const std::vector<const GlobalOperator *> &seq : paths) {
                if (counter > 1)  {
                    json << ",\n";
                }
                json << "  [";

                std::cout << "        " << "---sequence-" << counter << "---" << std::endl;
                if (seq.empty()) {
                    std::cout << "            <empty-sequence>" << std::endl;
                } else {
                    for (unsigned i = 0; i < seq.size(); i++) {
                        json << (i > 0 ? ", " : "")
                             << "\"" << seq[i]->get_name() << "\"";

                        std::cout << "            " << seq[i]->get_name() << std::endl;
                    }
                }

                counter++;

                json << "]";
            }
            paths.clear();
            num_states++;
        }
        num++;

        json << "]}";
    }
    std::cout << "---end-pareto-frontier---" << std::endl;

    if (c_silent) {
        std::cout.rdbuf(old);
    }

    std::cout << "(2OT) state(s) in Pareto frontier: " << num_states << std::endl;
    std::cout << "(2OT) registered state(s): " << g_outer_state_registry->size() <<
              std::endl;
    std::cout << "(2OT) expanded state(s): " << m_stat_expanded << std::endl;
    std::cout << "(2OT) evaluated state(s): " << m_stat_evaluated << std::endl;
    std::cout << "(2OT) generated state(s): " << m_stat_generated << std::endl;
    std::cout << "(2OT) pruned successor(s): " << m_stat_pruned_successors <<
              std::endl;
    std::cout << "(2OT) state(s) in open list: " << m_stat_open << std::endl;
    std::cout << "(2OT) state(s) expanded until last f-layer: " <<
              m_stat_expanded_last_f_layer << std::endl;
    std::cout << "(2OT) inner searches: " << m_stat_inner_searches << std::endl;
    printf("(2OT) inner search time: %.4fs\n", m_stat_time_inner_search());


    json << "]";

    std::ofstream out;
    out.open("pareto_frontier.json");
    out << json.str();
    out.close();
}

void BestFirstSearch::print_statistic_line()
{
    if (m_stat_last_g != m_stat_current_g) {
        m_stat_last_g = m_stat_current_g;
        m_stat_expanded_last_f_layer = m_stat_expanded;
    }

    if (m_stat_last_printed_states * 2 <= m_stat_expanded
            || m_stat_last_printed_pareto != m_pareto_frontier.size()) {
        m_stat_last_printed_states = m_stat_expanded;
        m_stat_last_printed_pareto = m_pareto_frontier.size();
        force_print_statistic_line();
    }
}

void BestFirstSearch::force_print_statistic_line() const
{
    if (!m_pareto_frontier.empty()) {
        printf("[g=%d, P={(%d, %d)..(%d, %d)} (%zu), expanded=%zu, open=%zu, pruned=%zu, t=%.3fs]\n",
               m_stat_current_g,
               m_pareto_frontier.rbegin()->second.first,
               m_pareto_frontier.rbegin()->first,
               m_pareto_frontier.begin()->second.first,
               m_pareto_frontier.begin()->first,
               m_pareto_frontier.size(),
               m_stat_expanded,
               m_stat_open,
               m_stat_pruned_successors,
               g_timer());
    }
}

void BestFirstSearch::add_options_to_parser(OptionParser &parser)
{
    std::vector<std::string> open_lists;
    get_open_list_options(open_lists);
    parser.add_enum_option("open_list", open_lists, "", open_lists.back());

    parser.add_option<SuccessorPruningMethod *>("pruning_method", "", "",
            OptionFlags(false));

    parser.add_option<bool>("precompute_max_reward", "", "true");
    parser.add_option<bool>("silent", "", "false");
    parser.add_option<bool>("lazy", "", "false");
    parser.add_option<bool>("sleep_set", "", "true");

    SecondOrderTaskSearch::add_options_to_parser(parser);
}

}

static SearchEngine *_parse(OptionParser &parser)
{
    second_order_search::BestFirstSearch::add_options_to_parser(parser);
    Options opts = parser.parse();
    if (!parser.dry_run()) {
        return new second_order_search::BestFirstSearch(opts);
    }
    return NULL;
}

static Plugin<SearchEngine> _plugin("2otbfs", _parse);
