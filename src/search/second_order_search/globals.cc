
#include "globals.h"

#include "../globals.h"
#include "../utilities.h"
#include "../utils/timer.h"

#include <iostream>
#include <cstdio>
#include <algorithm>

namespace second_order_search
{

bool __is_initialized = false;

bool cond_comp_func(GlobalCondition cond1, GlobalCondition cond2)
{
    return cond1.var < cond2.var;
}

bool eff_comp_func(GlobalEffect eff1, GlobalEffect eff2)
{
    return eff1.var < eff2.var;
}

GlobalState g_outer_initial_state()
{
    return g_outer_state_registry->get_initial_state();
}

void adjust_var_indices_of_ops(std::vector<GlobalOperator> &operators,
                               const std::vector<int> &newvarids)
{
    for (GlobalOperator &op : operators) {
        for (auto &c : op.get_preconditions()) {
            c.var = newvarids[c.var];
        }
        for (auto &e : op.get_effects()) {
            e.var = newvarids[e.var];
        }
        std::sort(op.get_preconditions().begin(), op.get_preconditions().end(),
                  cond_comp_func);
        std::sort(op.get_effects().begin(), op.get_effects().end(), eff_comp_func);
    }
}

void preprocess_second_order_task()
{
    if (__is_initialized) {
        return;
    }
    __is_initialized = true;

    std::cout << "Preprocessing second order task [t=" << g_timer << "]" <<
              std::endl;

    std::cout << "Splitting operators ..." << std::endl;
    int inner_op_id = 0;
    int outer_op_id = 0;
    for (size_t op_no = 0; op_no < g_operators.size(); op_no++) {
        const std::string &op_name = g_operators[op_no].get_name();
        if (op_name.size() >= 7 && op_name.substr(0, 7).compare("attack_") == 0) {
            g_inner_operators.push_back(g_operators[op_no]);
            g_inner_operators.back().set_op_id(inner_op_id);
            inner_op_id++;
        } else if (op_name.size() >= 4 && op_name.substr(0, 4).compare("fix_") == 0) {
            g_outer_operators.push_back(g_operators[op_no]);
            g_outer_operators.back().set_cost2(0);
            g_outer_operators.back().set_conds_variable_name(g_outer_variable_name);
            g_outer_operators.back().set_effs_variable_name(g_outer_variable_name);
            g_outer_operators.back().set_scheme_id(0);
            g_outer_operators.back().set_op_id(outer_op_id);
            outer_op_id++;
        } else {
            std::cerr << "Could not assign " << op_name << " to INNER or OUTER!" <<
                      std::endl;
            exit(EXIT_INPUT_ERROR);
        }
    }
    g_operators.clear();

    std::cout << "Finding variable split ..." << std::endl;
    // size_t temp = 0;
    std::vector<bool> is_inner_var(g_variable_domain.size(), false);
    for (size_t op_no = 0; op_no < g_inner_operators.size(); op_no++) {
        const std::vector<GlobalEffect> &effects =
            g_inner_operators[op_no].get_effects();
        for (size_t i = 0; i < effects.size(); i++) {
            int var = effects[i].var;
            if (is_inner_var[var]) {
                continue;
            } else {
                is_inner_var[var] = true;
                // temp++;
            }
        }
    }
    // size_t num_inner_vars = g_variable_domain.size() - temp;

    std::cout << "Processing inner operators ..." << std::endl;
    for (size_t op_no = 0; op_no < g_inner_operators.size(); op_no++) {
        g_inner_operator_outer_conditions.emplace_back();
        GlobalOperator &op = g_inner_operators[op_no];
        std::vector<GlobalCondition> &conditions = op.get_preconditions();
        std::vector<GlobalCondition> &outer_var_pre =
            g_inner_operator_outer_conditions.back();
        size_t j = 0;
        for (size_t i = 0; i < conditions.size(); i++) {
            int var = conditions[i].var;
            if (is_inner_var[var]) {
                if (i != j) {
                    conditions[j] = conditions[i];
                }
                j++;
            } else {
                outer_var_pre.push_back(conditions[i]);
            }
        }
        while (conditions.size() > j) {
            conditions.pop_back();
        }
    }

    std::cout << "Creating new variable indices ..." << std::endl;
    std::vector<int> newvarids(g_variable_domain.size(), -1);
    size_t inner_var_id = 0;
    size_t outer_var_id = 0;
    for (size_t var = 0; var < g_variable_domain.size(); var++) {
        if (is_inner_var[var]) {
            if (var != inner_var_id) {
                g_variable_domain[inner_var_id] = g_variable_domain[var];
                g_variable_name[inner_var_id] = g_variable_name[var];
                g_fact_names[inner_var_id].swap(g_fact_names[var]);
                g_initial_state_data[inner_var_id] = g_initial_state_data[var];
            }
            newvarids[var] = inner_var_id++;
        } else {
            g_outer_variable_domain.push_back(g_variable_domain[var]);
            g_outer_variable_name.push_back(g_variable_name[var]);
            g_outer_fact_names.emplace_back();
            g_outer_fact_names.back().swap(g_fact_names[var]);
            g_outer_initial_state_data.push_back(g_initial_state_data[var]);
            newvarids[var] = outer_var_id++;
        }
    }
    g_variable_domain.resize(inner_var_id);
    g_variable_name.resize(inner_var_id);
    g_fact_names.resize(inner_var_id);
    g_initial_state_data.resize(inner_var_id);

    adjust_var_indices_of_ops(g_inner_operators, newvarids);
    adjust_var_indices_of_ops(g_outer_operators, newvarids);
    for (auto &conds : g_inner_operator_outer_conditions) {
        for (auto &c : conds)  {
            c.var = newvarids[c.var];
        }
        std::sort(conds.begin(), conds.end(), cond_comp_func);
    }

#ifndef NDEBUG
    // make sure everything is set correctly
    for (unsigned i = 0; i < g_inner_operators.size(); i++) {
        const GlobalOperator &op = g_inner_operators[i];
        assert(op.get_op_id() == (int) i);
        for (const GlobalCondition &c : op.get_preconditions()) {
            assert(c.var < (int) g_variable_domain.size()
                   && c.val < g_variable_domain[c.var]);
        }
        for (const GlobalEffect &e : op.get_effects()) {
            assert(e.var < (int) g_variable_domain.size()
                   && e.val < g_variable_domain[e.var]);
        }
    }
    for (unsigned i = 0; i < g_outer_operators.size(); i++) {
        const GlobalOperator &op = g_outer_operators[i];
        assert(op.get_op_id() == (int) i);
        for (const GlobalCondition &c : op.get_preconditions()) {
            assert(c.var < (int) g_outer_variable_domain.size()
                   && c.val < g_outer_variable_domain[c.var]);
        }
        for (const GlobalEffect &e : op.get_effects()) {
            assert(e.var < (int) g_outer_variable_domain.size()
                   && e.val < g_outer_variable_domain[e.var]);
        }
    }
    for (auto &conds : g_inner_operator_outer_conditions) {
        for (const GlobalCondition &c : conds) {
            assert(c.var < (int) g_outer_variable_domain.size()
                   && c.val < g_outer_variable_domain[c.var]);
        }
    }
#endif

    size_t j = 0;
    for (size_t i = 0; i < g_goal.size(); i++) {
        int var = g_goal[i].first;
        if (!is_inner_var[var]) {
            std::cerr << "There should be no goal defined for OUTER variables! "
                      << "Ignoring the goal var#" << var << "=" << g_goal[i].second << " ("
                      << g_outer_fact_names[newvarids[var]][g_goal[i].second] << ")"
                      << std::endl;
            //exit(EXIT_INPUT_ERROR);
        } else {
            g_goal[j].first = newvarids[g_goal[i].first];
            g_goal[j].second = g_goal[i].second;
            j++;
        }
    }
    g_goal.resize(j);

    delete g_state_packer;
    g_state_packer = new IntPacker(g_variable_domain);
    delete g_state_registry;
    g_state_registry = new StateRegistry(g_state_packer, g_initial_state_data);

    g_outer_state_packer = new IntPacker(g_outer_variable_domain);
    g_outer_state_registry = new StateRegistry(g_outer_state_packer,
            g_outer_initial_state_data);

    std::cout << "Creating new successor generators ..." << std::endl;
    delete g_successor_generator;
    g_successor_generator = NULL;
    g_outer_successor_generator = create_successor_generator_from_vector(
                                      g_outer_variable_domain,
                                      g_outer_operators);
    g_outer_inner_successor_generator = create_successor_generator_from_vector(
                                            g_outer_variable_domain,
                                            g_inner_operators,
                                            &g_inner_operator_outer_conditions);

    ///////
    std::cout << "Inner variables: " << g_variable_domain.size() << std::endl;
    std::cout << "Bytes per inner state: " << (g_state_packer->get_num_bins() *
              g_state_packer->get_bin_size_in_bytes()) << std::endl;
    std::cout << "Inner operators: " << g_inner_operators.size() << std::endl;

    std::cout << "Outer variables: " << g_outer_variable_domain.size() << std::endl;
    std::cout << "Bytes per outer state: " << (g_outer_state_packer->get_num_bins()
              * g_outer_state_packer->get_bin_size_in_bytes()) << std::endl;
    std::cout << "Outer operators: " << g_outer_operators.size() << std::endl;

    std::cout << "Done preprocessing second order task! [t=" << g_timer << "]" <<
              std::endl;
}


std::vector<int> g_outer_variable_domain;
std::vector<std::string> g_outer_variable_name;
std::vector<std::vector<std::string> > g_outer_fact_names;

std::vector<int> g_outer_initial_state_data;

std::vector<GlobalOperator> g_inner_operators;
std::vector<std::vector<GlobalCondition> > g_inner_operator_outer_conditions;
std::vector<GlobalOperator> g_outer_operators;

SuccessorGenerator *g_outer_successor_generator;
SuccessorGenerator *g_outer_inner_successor_generator;

IntPacker *g_outer_state_packer;
StateRegistry *g_outer_state_registry;

}
