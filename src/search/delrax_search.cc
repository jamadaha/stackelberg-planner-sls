
#include "delrax_search.h"

#include "globals.h"
#include "global_state.h"
#include "global_operator.h"

#include "utilities.h"

#include "option_parser.h"
#include "plugin.h"

#include <algorithm>
#include <cassert>
#include <vector>
#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <queue>

namespace delrax_search
{

struct Comparator {
    const std::vector<int> &layers;
    Comparator(const std::vector<int> &layers) : layers(layers) {}
    bool operator()(const unsigned &i, const unsigned &j) const
    {
        return layers[i] > layers[j];
    }
};

bool is_task_delete_free()
{
    for (unsigned var = 0; var < g_variable_domain.size(); var++) {
        if (g_variable_domain[var] > 2) {
            return false;
        }
    }
    std::vector<int> first(g_variable_domain.size(), -1);
    std::vector<bool> ineff(g_variable_domain.size());
    std::vector<bool> inpre(g_variable_domain.size());
    for (unsigned opi = 0; opi < g_operators.size(); opi++) {
        std::fill(ineff.begin(), ineff.end(), false);
        const GlobalOperator &op = g_operators[opi];
        for (const auto &e : op.get_effects()) {
            if (first[e.var] == -1) {
                first[e.var] = e.val == 0 ? 1 : 0;
            }
            if (first[e.var] == e.val) {
                std::cout << 1 << std::endl;
                return false;
            }
            ineff[e.var] = true;
        }
        std::fill(inpre.begin(), inpre.end(), false);
        for (const auto &p : op.get_preconditions()) {
            if (first[p.var] == -1 && !ineff[p.var]) {
                first[p.var] = p.val == 0 ? 1 : 0;
            }
            if (first[p.var] == p.val && !ineff[p.var]) {
                std::cout << 2 << std::endl;
                return false;
            }
            inpre[p.var] = true;
        }
        for (const auto &e : op.get_effects()) {
            if (!inpre[e.var]) {
                std::cout << 3 << std::endl;
                return false;
            }
        }
    }
    // for (const auto &g : g_goal) {
    //     if (first[g.first] == g.second) {
    //         std::cout << 4 << std::endl;
    //         return false;
    //     }
    // }
    return true;
}


DelRaxSearch::DelRaxSearch(const Options &opts)
    : ::SearchEngine(opts),
      c_preprocess(opts.get<bool>("preprocess"))
{
}

void DelRaxSearch::verify()
{
    ::verify_no_axioms_no_conditional_effects();
    if (!is_task_delete_free()) {
        std::cerr << "DelRax search can be only applied to delete-free problems!" <<
                  std::endl;
        exit_with(EXIT_CRITICAL_ERROR);
    }
}

void DelRaxSearch::initialize()
{
    if (initialized) {
        std::cout << "DelRax search already initialized... skip this!" << std::endl;
        return;
    }

    std::cout << "Initializing DelRax search ..." << std::endl;
    verify();

    // in the following we assume that (when used in fixaction search)
    //  -- no operators will be added to g_operators that are not contained
    //     in g_operators at this time
    //  -- the "attacker" variables remain unchanged through the entire
    //     fixaction search
    //  -- g_initial_state_data remains unchanged

    m_positive_values.resize(g_variable_domain.size(), -1);
    std::vector<bool> ineff(g_variable_domain.size());
    for (unsigned opi = 0; opi < g_operators.size(); opi++) {
        std::fill(ineff.begin(), ineff.end(), false);
        const GlobalOperator &op = g_operators[opi];
        for (const auto &e : op.get_effects()) {
            if (m_positive_values[e.var] == -1) {
                m_positive_values[e.var] = e.val;
            }
            assert(m_positive_values[e.var] == e.val);
            ineff[e.var] = true;
        }
        for (const auto &p : op.get_preconditions()) {
            if (!ineff[p.var]) {
                if (m_positive_values[p.var] == -1) {
                    m_positive_values[p.var] = p.val;
                }
                assert(m_positive_values[p.var] == p.val);
            }
        }
    }

    // for (const auto &g : g_goal) {
    //     if (m_positive_values[g.first] == -1) {
    //         m_positive_values[g.first] = g.second;
    //     }
    //     assert(m_positive_values[g.first] == g.second);
    // }

    m_reward.resize(g_variable_domain.size(), 0);
    for (unsigned var = 0; var < g_variable_domain.size(); var++) {
        if (m_positive_values[var] < 0) {
            // irrelevant variabls -- happens only in self generated output
            // files
            continue;
        }
        size_t i = g_fact_names[var][m_positive_values[var]].find("_");
        // std::cout << var << ": " << g_fact_names[var][m_positive_values[var]]
        //           << " -> i=" << i << "; compare=" <<
        //           (g_fact_names[var][m_positive_values[var]].compare(0, 11, "Atom reward") == 0)
        //           << std::endl;
        if (g_fact_names[var][m_positive_values[var]].compare(0, 11, "Atom reward") == 0
                && i != std::string::npos) {
            m_reward[var] = atoi(g_fact_names[var][m_positive_values[var]].substr(11,
                                 i - 11).c_str());
            // std::cout << "Rewarding " << g_fact_names[var][m_positive_values[var]] << ": "
            //           << m_reward[var] << std::endl;
            if (m_reward[var] > 0) {
                m_goal.push_back(var);
            }
        }
    }


    // m_open_variables.reserve(g_variable_domain.size());
    // m_achieved.resize(g_variable_domain.size());
    m_closed.resize(g_variable_domain.size());

    initialized = true;
}

void DelRaxSearch::reset()
{
    if (!initialized) {
        return;
    }

    m_achievers.resize(g_variable_domain.size());
    std::fill(m_achievers.begin(), m_achievers.end(), -1);

    m_layer.resize(g_variable_domain.size());
    std::fill(m_layer.begin(), m_layer.end(), -1);

    m_in_pre.clear();
    m_in_effect.clear();
    m_open_variables.clear();

    m_num_preconditions.resize(g_operators.size());
    m_in_effect.resize(g_operators.size());
    m_in_pre.resize(g_variable_domain.size() + 1);

    GlobalState init = g_initial_state();

    for (unsigned opi = 0; opi < g_operators.size(); opi++) {
        const GlobalOperator &op = g_operators[opi];
        for (const auto &e : op.get_effects()) {
            if (init[e.var] != e.val) {
                m_in_effect[opi].push_back(e.var);
            }
        }
        if (m_in_effect[opi].empty()) {
            continue;
        }
        bool skip = false;
        for (const auto &p : op.get_preconditions()) {
            if (m_positive_values[p.var] != p.val && init[p.var] != p.val) {
                skip = true;
                break;
            }
        }
        if (skip) {
            continue;
        }
        m_num_preconditions[opi] = 0;
        for (const auto &p : op.get_preconditions()) {
            if (m_positive_values[p.var] == p.val) {
                m_num_preconditions[opi]++;
                m_in_pre[p.var].push_back(opi);
            }
        }
        if (m_num_preconditions[opi] == 0) {
            m_num_preconditions[opi] = 1;
            m_in_pre[g_variable_domain.size()].push_back(opi);
        }
    }

    m_open_variables.push_back(g_variable_domain.size());
    for (unsigned var = 0; var < g_variable_domain.size(); var++) {
        if (m_positive_values[var] == init[var]) {
            m_open_variables.push_back(var);
            m_layer[var] = 0;
        }
    }

    m_relaxed_plan.clear();
    std::fill(m_closed.begin(), m_closed.end(), false);
}

SearchStatus DelRaxSearch::step()
{
    reset();

    int layer = 1;
    unsigned next = m_open_variables.size();
    unsigned num = 0;
    while (!m_open_variables.empty()) {
        if (num++ == next) {
            layer++;
            next = m_open_variables.size();
        }
        unsigned var = m_open_variables.front();
        m_open_variables.pop_front();
        for (const unsigned &op : m_in_pre[var]) {
            if (--m_num_preconditions[op] == 0) {
                for (const unsigned &var2 : m_in_effect[op]) {
                    if (m_layer[var2] == -1) {
                        m_achievers[var2] = op;
                        m_layer[var2] = layer;
                        m_open_variables.push_back(var2);
                    }
                }
            }
        }
    }

    std::priority_queue<unsigned, std::vector<unsigned>, Comparator> trav
        = std::priority_queue<unsigned, std::vector<unsigned>, Comparator>(Comparator(
                    m_layer));
    for (const unsigned &g : m_goal) {
        if (m_layer[g] >= 0) {
            trav.push(g);
        }
    }
    if (trav.empty()) {
        return FAILED;
    }
    GlobalState init = g_initial_state();
    while (!trav.empty()) {
        unsigned x = trav.top();
        trav.pop();
        if (m_layer[x] == 0) {
            break;
        }
        if (m_closed[x]) {
            continue;
        }
        assert(m_achievers[x] < g_operators.size());
        unsigned op = m_achievers[x];
        m_relaxed_plan.push_back(&g_operators[op]);
        for (const auto &p : g_operators[op].get_preconditions()) {
            if (init[p.var] != p.val) {
                assert(m_positive_values[p.var] == p.val);
                assert(m_layer[p.var] > 0);
                trav.push(p.var);
            }
        }
        for (const unsigned &v : m_in_effect[op]) {
            m_closed[v] = true;
        }
    }
    std::reverse(m_relaxed_plan.begin(), m_relaxed_plan.end());
    set_plan(m_relaxed_plan);

    // std::cout << "Reward: " << calculate_plan_cost() << std::endl;

    return SOLVED;
}

int DelRaxSearch::calculate_plan_cost() const
{
    int res = 0;
    for (const unsigned &var : m_goal) {
        if (m_layer[var] >= 0) {
            res += m_reward[var];
        }
    }
    return -res;
}

void DelRaxSearch::add_options_to_parser(OptionParser &parser)
{
    parser.add_option<bool>("preprocess", "", "false");
    ::SearchEngine::add_options_to_parser(parser);
}

}

static SearchEngine *_parse(OptionParser &parser)
{
    delrax_search::DelRaxSearch::add_options_to_parser(parser);
    Options opts = parser.parse();
    if (!parser.dry_run()) {
        return new delrax_search::DelRaxSearch(opts);
    }
    return NULL;
}

static Plugin<SearchEngine> _plugin("delrax", _parse);
