
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
#include <deque>

#include <sstream>
#include <fstream>

namespace second_order_search
{

DepthFirstSearch::DepthFirstSearch(const Options &opts)
    : SecondOrderTaskSearch(opts),
      c_relevance_pruning(opts.get<bool>("relevance_pruning")),
      m_pruning_method(opts.contains("pruning_method") ?
                       opts.get<SuccessorPruningMethod *>("pruning_method") : NULL),
      m_max_reward(MAX_REWARD),
      m_cutoff(std::numeric_limits<int>::max())
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
    if (m_pruning_method != NULL) {
        m_pruning_method->initialize();
    }

    if (c_relevance_pruning) {
        const std::vector<int> &rew = get_inner_variable_rewards();
        for (size_t i = 0; i < rew.size(); i++) {
            if (rew[i] != 0) {
                m_inner_reward_variables.push_back(i);
            }
        }

        const std::vector<int> &pval = get_inner_positive_values();
        std::vector<std::vector<size_t> > in_pre_of(g_variable_domain.size());
        for (size_t i = 0; i < g_inner_operators.size(); i++) {
            for (const auto &p : g_inner_operators[i].get_preconditions()) {
                if (pval[p.var] == p.val) {
                    in_pre_of[p.var].push_back(i);
                }
            }
        }
        std::vector<std::vector<bool> > inner_op_relevance(g_inner_operators.size());
        std::deque<size_t> q;
        std::vector<bool> closed(g_inner_operators.size());
        for (size_t i = 0; i < g_inner_operators.size(); i++) {
            inner_op_relevance[i].resize(g_variable_domain.size(), false);
            std::fill(closed.begin(), closed.end(), false);
            closed[i] = true;
            q.push_back(i);
            while (!q.empty()) {
                size_t op = q.front();
                q.pop_front();
                for (const auto &e : g_inner_operators[op].get_effects()) {
                    assert(pval[e.var] == e.val);
                    if (!inner_op_relevance[i][e.var]) {
                        inner_op_relevance[i][e.var] = true;
                        for (const size_t &succ : in_pre_of[e.var]) {
                            if (!closed[succ]) {
                                closed[succ] = true;
                                q.push_back(succ);
                            }
                        }
                    }
                }
            }
        }
        std::vector<std::vector<size_t> >().swap(in_pre_of);

        std::vector<std::vector<std::vector<size_t> > > in_pre
        (g_outer_variable_domain.size());
        std::vector<std::vector<std::vector<size_t> > > neg_in_inner_pre(
            g_outer_variable_domain.size());
        for (size_t i = 0; i < g_outer_variable_domain.size(); i++) {
            in_pre[i].resize(g_outer_variable_domain[i]);
            neg_in_inner_pre[i].resize(g_outer_variable_domain[i]);
        }
        for (size_t i = 0; i < g_outer_operators.size(); i++) {
            for (const auto &p : g_outer_operators[i].get_preconditions()) {
                in_pre[p.var][p.val].push_back(i);
            }
        }
        for (size_t i = 0; i < g_inner_operator_outer_conditions.size(); i++) {
            for (const auto &p : g_inner_operator_outer_conditions[i]) {
                for (int val = 0; val < g_outer_variable_domain[p.var]; val++) {
                    if (val != p.val) {
                        neg_in_inner_pre[p.var][val].push_back(i);
                    }
                }
            }
        }
        m_outer_op_relevance.resize(g_outer_operators.size());
        closed.resize(g_outer_operators.size());
        for (size_t i = 0; i < g_outer_operators.size(); i++) {
            m_outer_op_relevance[i].resize(g_variable_domain.size(), false);
            std::fill(closed.begin(), closed.end(), false);
            closed[i] = true;
            q.push_back(i);
            while (!q.empty()) {
                size_t op = q.front();
                q.pop_front();
                for (const auto &e : g_outer_operators[op].get_effects()) {
                    for (const size_t &succ : in_pre[e.var][e.val]) {
                        if (!closed[succ]) {
                            closed[succ] = true;
                            q.push_back(succ);
                        }
                    }
                    for (const size_t &inner_op : neg_in_inner_pre[e.var][e.val]) {
                        const std::vector<bool> &rele = inner_op_relevance[inner_op];
                        for (const size_t &p : m_inner_reward_variables) {
                            m_outer_op_relevance[i][p] = m_outer_op_relevance[i][p] || rele[p];
                        }
                    }
                }
            }
        }
    }
}

bool DepthFirstSearch::_insert_into_pareto_frontier(
    const int &reward,
    const int &g,
    const StateID &state)
{
    if (insert_into_pareto_frontier(reward, g, state)) {
        std::vector<const GlobalOperator *> &path = m_paths[state];
        path.clear();
        path.insert(path.end(), m_current_path.begin(), m_current_path.end());
        return true;
    }
    return false;
}

void DepthFirstSearch::expand(const GlobalState &state,
                              IntPacker::Bin *counter,
                              std::vector<size_t> &sleep,
                              std::vector<size_t> relevant,
                              int pr,
                              int pg)
{
    m_stat_expanded++;

    size_t r_i;
    size_t r_j;
    if (c_relevance_pruning) {
        bool flip = true;
        while (true) {
            r_j = 0;
            for (r_i = 0; r_i < relevant.size(); r_i++) {
                if (m_counter_packer->get(counter, relevant[r_i]) > 0) {
                    if (r_i != r_j) {
                        relevant[r_j] = relevant[r_i];
                    }
                    r_j++;
                }
            }
            relevant.resize(r_j);

            if (flip && relevant.empty()) {
                flip = false;
                relevant.insert(relevant.end(),
                                m_inner_reward_variables.begin(),
                                m_inner_reward_variables.end());
            } else {
                break;
            }
        }
        assert(!relevant.empty() || get_reward(counter) == MAX_REWARD);
    }

    std::vector<const GlobalOperator *> aops;
    g_outer_successor_generator->generate_applicable_ops(state, aops);
    if (m_pruning_method != NULL) {
        set_inner_plan(counter);
        m_pruning_method->prune_successors(state, g_plan, aops);
        g_plan.clear();
    }
    int succ_g;
    int succ_r;
    size_t aops_size = aops.size();
    std::vector<size_t> removed;
    size_t i = 0;
    while (i < aops_size) {
        size_t id = aops[i]->get_op_id();
        if (c_relevance_pruning) {
            const std::vector<bool> &is_relevant_for = m_outer_op_relevance[id];
            r_j = 0;
            for (r_i = 0; r_i < relevant.size(); r_i++) {
                const size_t &p = relevant[r_i];
                if (is_relevant_for[p]) {
                    if (r_i != r_j) {
                        relevant[r_j] = relevant[r_i];
                    }
                    r_j++;
                } else {
                    removed.push_back(p);
                }
            }
            if (r_j == 0) {
                removed.clear();
                // std::cout << " --> pruning (" << aops[i]->get_op_id() << ") "
                //           << aops[i]->get_name()
                //           << std::endl;
                if (aops_size - 1 != i) {
                    aops[i] = aops[aops_size - 1];
                }
                aops_size--;
                continue;
            }
            relevant.resize(r_j);
        }
        if (sleep[id]++ == 0) {
            m_stat_generated++;
            GlobalState succ = g_outer_state_registry->get_successor_state(state, *aops[i]);
            succ_g = pg + get_adjusted_cost(*aops[i]);
            if (succ_g <= m_cutoff) {
                // std::cout << "> (" << aops[i]->get_op_id() << ") "
                //           << aops[i]->get_name()
                //           << std::endl;
                m_stat_evaluated++;
                IntPacker::Bin *succ_counter;
#ifndef NDEBUG
                succ_r = pr
                         - compute_reward_difference(
                             succ,
                             counter,
                             *aops[i],
                             succ_counter);
#else
                succ_r = pr
                         - compute_reward_difference(counter,
                                                     *aops[i],
                                                     succ_counter);
#endif
                m_current_path.push_back(aops[i]);
                if (_insert_into_pareto_frontier(succ_r,
                                                 succ_g,
                                                 succ.get_id())
                        && succ_r == m_max_reward) {
                    m_cutoff = succ_g;
                } else if (succ_r == pr) {
                    expand(succ, succ_counter, sleep, relevant, succ_r, succ_g);
                } else {
                    expand(succ, succ_counter, sleep, m_inner_reward_variables, succ_r, succ_g);
                }
                m_current_path.pop_back();
                delete[](succ_counter);
                // std::cout << "< (" << aops[i]->get_op_id() << ") "
                //           << aops[i]->get_name()
                //           << std::endl;
            }
        }
        if (c_relevance_pruning && !removed.empty()) {
            relevant.insert(relevant.end(), removed.begin(), removed.end());
            removed.clear();
        }
        i++;
    }
    for (size_t i = 0; i < aops_size; i++) {
        sleep[aops[i]->get_op_id()]--;
    }
    print_statistic_line();
}

SearchStatus DepthFirstSearch::step()
{
    GlobalState init = g_outer_initial_state();
    IntPacker::Bin *counter;
    int r = get_initial_state_reward(counter);
    int g = 0;
    _insert_into_pareto_frontier(r, g, init.get_id());
    if (m_cutoff > 0) {
        std::vector<size_t> sleep(g_outer_operators.size(), 0);
        std::vector<size_t> relevant;
        expand(init, counter, sleep, relevant, r, g);
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
    parser.add_option<bool>("relevance_pruning", "", "false");
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
