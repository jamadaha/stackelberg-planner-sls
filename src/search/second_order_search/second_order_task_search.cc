#include "second_order_task_search.h"

#include "globals.h"

#include "../option_parser.h"
#include "../globals.h"

#include "../successor_generator.h"

#include "../utilities.h"

#include "../delrax_search.h"

#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <deque>

#include <cstring>

namespace second_order_search
{

SecondOrderTaskSearch::SecondOrderTaskSearch(const Options &opts)
    : SearchEngine(opts),
      c_incremental_rpg(opts.get<bool>("incremental_rpg")),
      m_inner_search(dynamic_cast<delrax_search::DelRaxSearch *>
                     (opts.get<SearchEngine *>("inner_search")))
{
}

void SecondOrderTaskSearch::initialize()
{
    std::cout << "Initializing 2OT search ..." << std::endl;
    preprocess_second_order_task();

    // *HACK* make sure inner search is properly initialized
    run_inner_search(g_outer_initial_state());

    if (!c_incremental_rpg) {
        return;
    }

    // copying data structures ...
    std::copy(m_inner_search->get_reward().begin(),
              m_inner_search->get_reward().end(),
              std::back_inserter(m_rewards));
    const std::vector<int> &pv = m_inner_search->get_positive_values();
    // TODO copy/compute goal i.e. facts with non-zero reward ?
    m_arcs.resize(g_variable_domain.size() + g_inner_operators.size());
    m_inv_arcs.resize(g_variable_domain.size() + g_inner_operators.size());
    for (unsigned op = 0; op < g_inner_operators.size(); op++) {
        for (const auto &p : g_inner_operators[op].get_preconditions()) {
            if (pv[p.var] == p.val) {
                m_arcs[p.var].push_back(g_variable_domain.size() + op);
                m_inv_arcs[g_variable_domain.size() + op].push_back(p.var);
            }
        }
        for (const auto &e : g_inner_operators[op].get_effects()) {
            assert(pv[e.var] == e.val);
            m_arcs[g_variable_domain.size() + op].push_back(e.var);
            m_inv_arcs[e.var].push_back(g_variable_domain.size() + op);
        }
    }

    // computing outer to inner op map
    std::vector<std::vector<std::vector<unsigned> > > fact_neg_by(
        g_outer_variable_domain.size());
    for (unsigned var = 0; var < g_outer_variable_domain.size(); var++) {
        fact_neg_by[var].resize(g_outer_variable_domain[var]);
    }

    std::vector<int> prec(g_outer_variable_domain.size());
    for (unsigned opi = 0; opi < g_outer_operators.size(); opi++) {
        std::fill(prec.begin(), prec.end(), -1);
        const GlobalOperator &op = g_outer_operators[opi];
        for (const auto &p : op.get_preconditions()) {
            prec[p.var] = p.val;
        }
        for (const auto &e : op.get_effects()) {
            if (prec[e.var] == -1) {
                for (int val = 0; val < g_outer_variable_domain[e.var]; val++) {
                    if (val != e.val) {
                        fact_neg_by[e.var][val].push_back(opi);
                    }
                }
            } else {
                fact_neg_by[e.var][prec[e.var]].push_back(opi);
            }
        }
    }

    m_outer_to_inner_operator.resize(g_outer_operators.size());
    std::vector<bool> duplicate(g_outer_operators.size());
    assert(g_inner_operator_outer_conditions.size() == g_inner_operators.size());
    for (unsigned opi = 0; opi < g_inner_operator_outer_conditions.size(); opi++) {
        std::fill(duplicate.begin(), duplicate.end(), false);
        for (const auto &c : g_inner_operator_outer_conditions[opi]) {
            for (unsigned x : fact_neg_by[c.var][c.val]) {
                if (!duplicate[x]) {
                    duplicate[x] = true;
                    m_outer_to_inner_operator[x].push_back(g_variable_domain.size() + opi);
                    assert(x < m_outer_to_inner_operator.size());
                    assert(g_outer_operators[x].get_op_id() == (int) x);
                    assert(m_outer_to_inner_operator[x].back() < m_arcs.size());
                }
            }
        }
    }

#ifndef NDEBUG
    for (unsigned x = 0; x < m_arcs.size(); x++) {
        for (const unsigned &y : m_arcs[x]) {
            assert(x < g_variable_domain.size() || y < g_variable_domain.size());
            assert(x >= g_variable_domain.size() || y >= g_variable_domain.size());
        }
    }
    for (unsigned i = 0; i < g_inner_operators.size(); i++) {
        unsigned nump = 0;
        for (const auto &c : g_inner_operators[i].get_preconditions()) {
            if (pv[c.var] == c.val) {
                nump++;
            }
        }
        assert(nump <= 1);
    }
    // verify acyclicity
    std::vector<bool> onstack;
    std::vector<int> ids(m_arcs.size(), -1);
    std::deque<unsigned> q;
    bool loop = false;
    std::vector<unsigned> loop_;
    for (unsigned x = 0; !loop && x < m_arcs.size(); x++) {
        if (ids[x] == -1) {
            q.push_back(x);
            while (!q.empty()) {
                unsigned y = q.back();
                q.pop_back();
                if (ids[y] == -1) {
                    if (loop) {
                        continue;
                    }
                    ids[y] = onstack.size();
                    onstack.push_back(true);
                }
                if (loop) {
                    loop_.push_back(y);
                    continue;
                }
                bool done = true;
                for (unsigned z : m_arcs[y]) {
                    assert(z < m_arcs.size());
                    if (ids[z] == -1) {
                        q.push_back(y);
                        q.push_back(z);
                        done = false;
                        break;
                    } else {
                        assert(ids[z] >= 0 && ids[z] < (int) onstack.size());
                        if (onstack[ids[z]]) {
                            loop = true;
                            std::cerr << "Cannot handle relaxed planning graphs with cycles!" << std::endl;
                            loop_.push_back(z);
                            loop_.push_back(y);
                            break;
                        }
                    }
                }
                if (done) {
                    onstack[ids[y]] = false;
                }
            }
        }
    }
    if (loop) {
        std::reverse(loop_.begin(), loop_.end());
        std::cerr << "---begin-cycle---" << std::endl;
        for (const unsigned &x : loop_) {
            if (x < g_variable_domain.size()) {
                std::cerr << g_fact_names[x][pv[x]] << std::endl;
            } else {
                std::cerr << g_inner_operators[x - g_variable_domain.size()].get_name() <<
                          std::endl;
            }
        }
        std::cerr << "---end-cycle---" << std::endl;
        exit_with(EXIT_CRITICAL_ERROR);
    }
#endif

    // computing counter ranges
    // the outer condition of all inner operators are satisfied in the initial
    // outer state
    rgraph_exploration(g_outer_initial_state(), m_initial_state_counter);

    std::vector<int> range(m_initial_state_counter);
    for (unsigned i = 0; i < range.size(); i++) {
        range[i] = range[i] + 1;
    }
    assert(range.size() == m_arcs.size());

    m_counter_packer = new IntPacker(range);

    m_closed.resize(g_variable_domain.size(), false);

    std::cout << "Bytes per RPG counter: " << (m_counter_packer->get_num_bins() *
              sizeof(IntPacker::Bin)) << std::endl;
}

void SecondOrderTaskSearch::rgraph_exploration(
    const GlobalState &state,
    std::vector<int> &res)
{
    res.clear();
    res.resize(m_arcs.size(), 0);

    std::deque<unsigned> open;

    g_outer_inner_successor_generator->generate_applicable_ops(
        state,
        m_available_operators);

    GlobalState inner_init = g_initial_state();
    for (unsigned i = 0; i < m_available_operators.size(); i++) {
        if (m_available_operators[i]->is_applicable(inner_init)) {
            res[m_available_operators[i]->get_op_id() + g_variable_domain.size()] = 1;
            open.push_back(m_available_operators[i]->get_op_id() +
                           g_variable_domain.size());
        }
    }

    while (!open.empty()) {
        unsigned s = open.front();
        open.pop_front();
        for (const unsigned &t : m_arcs[s]) {
            if (++res[t] == 1) {
                open.push_back(t);
            }
        }
    }

    m_available_operators.clear();
}


int SecondOrderTaskSearch::get_reward(const std::vector<int> &counter) const
{
    int res = 0;
    for (unsigned var = 0; var < g_variable_domain.size(); var++) {
        if (counter[var] > 0) {
            res += m_rewards[var];
        }
    }
    return -res;
}

int SecondOrderTaskSearch::get_reward(const IntPacker::Bin *const &counter)
const
{
    int res = 0;
    for (unsigned var = 0; var < g_variable_domain.size(); var++) {
        if (m_counter_packer->get(counter, var) > 0) {
            res += m_rewards[var];
        }
    }
    return -res;
}

int SecondOrderTaskSearch::get_initial_state_reward(IntPacker::Bin *&result)
const
{
    result = new IntPacker::Bin[m_counter_packer->get_num_bins()];
    for (unsigned i = 0; i < m_initial_state_counter.size(); i++) {
        m_counter_packer->set(result, i, m_initial_state_counter[i]);
    }
    return get_reward(result);
}

int SecondOrderTaskSearch::compute_reward_difference(
    const IntPacker::Bin *const &parent,
    const GlobalOperator &op,
    IntPacker::Bin *&res)
{
    res = new IntPacker::Bin[m_counter_packer->get_num_bins()];
    memcpy(res, parent, m_counter_packer->get_num_bins() * sizeof(IntPacker::Bin));

    std::deque<unsigned> open;
    assert(m_outer_to_inner_operator.size() == g_outer_operators.size());
    assert(op.get_op_id() < (int) m_outer_to_inner_operator.size());
    for (const unsigned &x : m_outer_to_inner_operator[op.get_op_id()]) {
        assert(x < m_arcs.size());
        if (m_counter_packer->get(res, x) > 0) {
            assert(x >= m_rewards.size());
            assert(m_counter_packer->get(res, x) == 1);
            m_counter_packer->set(res, x, 0);
            open.push_back(x);
        }
    }
    int diff = 0;
    int c;
    while (!open.empty()) {
        unsigned s = open.front();
        open.pop_front();
        for (const unsigned &t : m_arcs[s]) {
            c = m_counter_packer->get(res, t);
            if (c > 0) {
                c--;
                m_counter_packer->set(res, t, c);
                if (c == 0) {
                    open.push_back(t);
                    if (t < m_rewards.size()) {
                        diff += m_rewards[t];
                    }
                }
            }
        }
    }

    return -diff;
}

void SecondOrderTaskSearch::extract_inner_plan(const IntPacker::Bin *const
        &counter,
        std::vector<const GlobalOperator *> &plan)
{
    std::fill(m_closed.begin(), m_closed.end(), false);
    std::deque<unsigned> open;
    for (unsigned var = 0; var < m_closed.size(); var++) {
        if (m_rewards[var] > 0 && m_counter_packer->get(counter, var) > 0) {
            open.push_back(var);
            m_closed[var] = true;
        }
    }
    while (!open.empty()) {
        unsigned s = open.front();
        open.pop_front();
        for (const unsigned &o : m_inv_arcs[s]) {
            if (m_counter_packer->get(counter, o) > 0) {
                for (const unsigned &t : m_inv_arcs[o]) {
                    if (!m_closed[t]) {
                        open.push_back(t);
                        m_closed[t] = true;
                    }
                }
                plan.push_back(&g_inner_operators[o - g_variable_domain.size()]);
                break;
            }
        }
    }

    std::reverse(plan.begin(), plan.end());
}

int SecondOrderTaskSearch::run_inner_search(const GlobalState &state)
{
    g_outer_inner_successor_generator->generate_applicable_ops(
        state,
        m_available_operators);
    g_operators.clear();
    for (const GlobalOperator *op : m_available_operators) {
        g_operators.push_back(*op);
    }
    m_available_operators.clear();

    std::streambuf *old = std::cout.rdbuf(); // <-- save
    std::stringstream ss;
    std::cout.rdbuf(ss.rdbuf());        // <-- redirect
    m_inner_search->reset();
    m_inner_search->search();
    int res = MAX_REWARD;
    if (m_inner_search->found_solution()) {
        res = m_inner_search->calculate_plan_cost();
    }
    std::cout.rdbuf(old);   			// <-- restore

    return res;
}

void SecondOrderTaskSearch::add_options_to_parser(OptionParser &parser)
{
    parser.add_option<SearchEngine *>("inner_search", "", "delrax");
    parser.add_option<bool>("incremental_rpg", "", "true");
    SearchEngine::add_options_to_parser(parser);
}

}
