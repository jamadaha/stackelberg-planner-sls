
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
#include <algorithm>

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
      m_state_rewards(StateInfo(std::numeric_limits<int>::min(), -1
#if STORE_POLICY_IN_INFO
                                , NULL, StateID::no_state
#endif
                               )),
      m_max_reward(MAX_REWARD),
      m_cutoff(std::numeric_limits<int>::max()),
      m_pruning_method(opts.contains("pruning_method") ?
                       opts.get<SuccessorPruningMethod *>("pruning_method") : NULL)
{
    m_stat_expanded = 0;
    m_stat_updated = 0;
    m_stat_evaluated = 1;
    m_stat_generated = 1;
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
    const StateInfo &val,
    const StateID &state)
{
    if (insert_into_pareto_frontier(val.r, val.g, state)) {
#if !STORE_POLICY_IN_INFO
        std::vector<const GlobalOperator *> &path = m_paths[state];
        path.clear();
        path.insert(path.end(), m_current_path.begin(), m_current_path.end());
#endif
        return true;
    }
    return false;
}

bool DepthFirstSearch::update_g_values(const GlobalState &state,
#if STORE_POLICY_IN_INFO
                                       const StateID &p,
                                       const GlobalOperator *op,
#endif
                                       int newg)
{
    StateInfo &val = m_state_rewards[state];
    if (newg >= val.g) {
        return false;
    }

    m_stat_updated++;

    val.g = newg;
#if STORE_POLICY_IN_INFO
    val.p = p;
    val.p_op = op;
#endif

    if (_insert_into_pareto_frontier(val, state.get_id())
            && val.r == m_max_reward) {
        m_cutoff = val.g;
        return true;
    }

    std::vector<const GlobalOperator *> aops;
    g_outer_successor_generator->generate_applicable_ops(state, aops);
    for (size_t i = 0; i < aops.size(); i++) {
#if !STORE_POLICY_IN_INFO
        m_current_path.push_back(aops[i]);
#endif
        GlobalState succ = g_outer_state_registry->get_successor_state(state, *aops[i]);
        update_g_values(succ,
#if STORE_POLICY_IN_INFO
                        state.get_id(), aops[i],
#endif
                        newg + get_adjusted_cost(*aops[i]));
#if !STORE_POLICY_IN_INFO
        m_current_path.pop_back();
#endif
    }

    return true;
}

void DepthFirstSearch::expand(const GlobalState &state,
                              IntPacker::Bin *counter,
                              std::vector<size_t> &sleep)
{
    m_stat_expanded++;
    // m_state_rewards[state] has been set
    const StateInfo &stats = m_state_rewards[state];
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
        m_stat_generated++;
        GlobalState succ = g_outer_state_registry->get_successor_state(state, *aops[i]);
        StateInfo &succ_stats = m_state_rewards[succ];
        if (succ_stats.g == -1) {
            // is new
            succ_stats.g = stats.g + get_adjusted_cost(*aops[i]);
#if STORE_POLICY_IN_INFO
            succ_stats.p = state.get_id();
            succ_stats.p_op = aops[i];
#endif
            if (succ_stats.g <= m_cutoff) {
                m_stat_evaluated++;
                IntPacker::Bin *succ_counter;
#ifndef NDEBUG
                succ_stats.r = stats.r
                               - compute_reward_difference(
                                   succ,
                                   counter,
                                   *aops[i],
                                   succ_counter);
#else
                succ_stats.r = stats.r
                               - compute_reward_difference(counter,
                                       *aops[i],
                                       succ_counter);
#endif
#if !STORE_POLICY_IN_INFO
                m_current_path.push_back(aops[i]);
#endif
                if (_insert_into_pareto_frontier(succ_stats,
                                                 succ.get_id())
                        && succ_stats.r == m_max_reward) {
                    m_cutoff = succ_stats.g;
                } else {
                    expand(succ, succ_counter, sleep);
                }
#if !STORE_POLICY_IN_INFO
                m_current_path.pop_back();
#endif
                delete[](succ_counter);
            }
        } else {
            update_g_values(succ,
#if STORE_POLICY_IN_INFO
                            state.get_id(), aops[i],
#endif
                            stats.g + get_adjusted_cost(*aops[i]));
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
    StateInfo &info = m_state_rewards[init];
    info.r = get_initial_state_reward(counter);
    info.g = 0;
    _insert_into_pareto_frontier(info, init.get_id());
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
#if STORE_POLICY_IN_INFO
    paths.emplace_back();
    std::vector<const GlobalOperator *> &path = paths.back();
    StateID s = state;
    while (s != StateID::no_state) {
        const StateInfo &info = m_state_rewards[s];
        path.push_back(info.p_op);
        s = info.p;
    }
    path.pop_back();
    std::reverse(path.begin(), path.end());
#else
    assert(m_paths.count(state));
    paths.push_back(m_paths[state]);
#endif
}

void DepthFirstSearch::add_options_to_parser(OptionParser &parser)
{
    parser.add_option<SuccessorPruningMethod *>("pruning_method", "", "",
            OptionFlags(false));
    SecondOrderTaskSearch::add_options_to_parser(parser);
}

void DepthFirstSearch::statistics() const
{
    std::cout << "(2OT) registered state(s): " << g_outer_state_registry->size() <<
              std::endl;
    std::cout << "(2OT) expanded state(s): " << m_stat_expanded << std::endl;
    std::cout << "(2OT) evaluated state(s): " << m_stat_evaluated << std::endl;
    std::cout << "(2OT) generated state(s): " << m_stat_generated << std::endl;
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
