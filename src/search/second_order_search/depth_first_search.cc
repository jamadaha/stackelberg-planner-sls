
#include "depth_first_search.h"

#include "globals.h"
#include "successor_pruning_method.h"

#include "../state_registry.h"
#include "../successor_generator.h"

#include "../global_state.h"
#include "../global_operator.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "../globals.h"
#include "../timer.h"

#include "../int_packer.h"

#include "../utilities.h"

#include <limits>
#include <string>
#include <iostream>
#include <cstdio>

#include <sstream>
#include <fstream>

namespace second_order_search
{

// CallStackElement::CallStackElement(const GlobalState &state)
//     : state_id(state.get_id()), i(0), counter(NULL)
// {
//     g_outer_successor_generator->generate_applicable_ops(state, aops);
// }
//
// CallStackElement::CallStackElement(const GlobalState &state,
//                                    const std::vector<bool> &sleep,
//                                    IntPacker::Bin *counter)
//     : CallStackElement(state),
//       sleep(sleep)
// {
// }

DepthFirstSearch::DepthFirstSearch(const Options &opts)
    : SecondOrderTaskSearch(opts),
      m_state_rewards(std::pair<int, int>(std::numeric_limits<int>::min(), -1)),
      m_max_reward(std::numeric_limits<int>::max()),
      m_cutoff(std::numeric_limits<int>::max()),
      m_pruning_method(opts.contains("pruning_method") ?
                       opts.get<SuccessorPruningMethod *>("pruning_method") : NULL)
{
    m_stat_expanded = 0;
    m_stat_updated = 0;
    m_stat_evaluated = 1;
    m_stat_last_printed = 0;
}

void DepthFirstSearch::initialize()
{
    SecondOrderTaskSearch::initialize();
    // m_stack.emplace_back(g_outer_initial_state());
    if (m_pruning_method != NULL) {
        m_pruning_method->initialize();
    }
}

bool DepthFirstSearch::_insert_into_pareto_frontier(
    const std::pair<int, int> &val,
    const StateID &state)
{
    if (insert_into_pareto_frontier(val.first, val.second, state)) {
        std::vector<const GlobalOperator *> &path = m_paths[state];
        path.clear();
        path.insert(path.end(), m_current_path.begin(), m_current_path.end());
        return true;
    }
    return false;
}

bool DepthFirstSearch::update_g_values(const GlobalState &state, int newg)
{
    std::pair<int, int> &val = m_state_rewards[state];
    if (newg >= val.second) {
        return false;
    }
    val.second = newg;
    m_stat_updated++;
    if (_insert_into_pareto_frontier(val, state.get_id())
            && val.first == m_max_reward) {
        m_cutoff = val.second;
        return true;
    }
    std::vector<const GlobalOperator *> aops;
    g_outer_successor_generator->generate_applicable_ops(state, aops);
    for (size_t i = 0; i < aops.size(); i++) {
        m_current_path.push_back(aops[i]);
        GlobalState succ = g_outer_state_registry->get_successor_state(state, *aops[i]);
        update_g_values(succ, newg + get_adjusted_cost(*aops[i]));
        m_current_path.pop_back();
    }
    return true;
}

void DepthFirstSearch::expand(const GlobalState &state,
                              IntPacker::Bin *counter,
                              std::vector<size_t> &sleep)
{
    m_stat_expanded++;
    // m_state_rewards[state] has been set
    const std::pair<int, int> &stats = m_state_rewards[state];
    std::vector<const GlobalOperator *> aops;
    g_outer_successor_generator->generate_applicable_ops(state, aops);
    if (m_pruning_method != NULL) {
        set_inner_plan(counter);
        m_pruning_method->prune_successors(state, g_plan, aops);
        g_plan.clear();
    }
    for (size_t i = 0; i < aops.size(); i++) {
        size_t id = aops[i]->get_op_id();
        if (sleep[id]++) {
            continue;
        }
        GlobalState succ = g_outer_state_registry->get_successor_state(state, *aops[i]);
        std::pair<int, int> &succ_stats = m_state_rewards[succ];
        if (succ_stats.second == -1) {
            // is new
            succ_stats.second = stats.second + get_adjusted_cost(*aops[i]);
            if (succ_stats.second <= m_cutoff) {
                m_stat_evaluated++;
                IntPacker::Bin *succ_counter;
#ifndef NDEBUG
                succ_stats.first = stats.first
                                   - compute_reward_difference(
                                       succ,
                                       counter,
                                       *aops[i],
                                       succ_counter);
#else
                succ_stats.first = stats.first
                                   - compute_reward_difference(counter,
                                           *aops[i],
                                           succ_counter);
#endif
                m_current_path.push_back(aops[i]);
                if (_insert_into_pareto_frontier(succ_stats,
                                                 succ.get_id())
                        && succ_stats.first == m_max_reward) {
                    m_cutoff = succ_stats.second;
                } else {
                    expand(succ, succ_counter, sleep);
                }
                m_current_path.pop_back();
                delete[](succ_counter);
            }
        } else {
            update_g_values(succ, stats.second + get_adjusted_cost(*aops[i]));
        }
    }
    for (size_t i = 0; i < aops.size(); i++) {
        sleep[aops[i]->get_op_id()]--;
    }
    print_statistic_line();
}

SearchStatus DepthFirstSearch::step()
{
    GlobalState init = g_outer_initial_state();
    IntPacker::Bin *counter;
    int r = get_initial_state_reward(counter);
    m_state_rewards[init] = std::pair<int, int>(r, 0);
    _insert_into_pareto_frontier(std::pair<int, int>(r, 0), init.get_id());
    if (m_cutoff > 0) {
        std::vector<size_t> sleep(g_outer_operators.size(), 0);
        expand(init, counter, sleep);
    }
    return SOLVED;
}

void DepthFirstSearch::print_statistic_line()
{
    if (!m_pareto_frontier.empty()
            && g_outer_state_registry->size() >= 2 * m_stat_last_printed) {
        m_stat_last_printed = g_outer_state_registry->size();
        printf("[P={(%d, %d)..(%d, %d)} (%zu), expanded=%zu, updated=%zu, registered=%zu, t=%.3fs]\n",
               m_pareto_frontier.rbegin()->second.first,
               m_pareto_frontier.rbegin()->first,
               m_pareto_frontier.begin()->second.first,
               m_pareto_frontier.begin()->first,
               m_pareto_frontier.size(),
               m_stat_expanded,
               m_stat_updated,
               g_outer_state_registry->size(),
               g_timer());
    }
}

void DepthFirstSearch::get_paths(const StateID &state,
                                 std::vector<std::vector<const GlobalOperator *> > &paths)
{
    assert(m_paths.count(state));
    paths.push_back(m_paths[state]);
}

void DepthFirstSearch::add_options_to_parser(OptionParser &parser)
{
    parser.add_option<SuccessorPruningMethod *>("pruning_method", "", "",
            OptionFlags(false));
    SecondOrderTaskSearch::add_options_to_parser(parser);
}

}

static SearchEngine *_parse(OptionParser &parser)
{
    second_order_search::DepthFirstSearch::add_options_to_parser(parser);
    Options opts = parser.parse();
    if (!parser.dry_run()) {
        return new second_order_search::DepthFirstSearch(opts);
    }
    return NULL;
}

static Plugin<SearchEngine> _plugin("2otdfs", _parse);
