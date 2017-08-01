/*
 * fixactionssearch.cpp
 *
 *  Created on: 12.12.2016
 *      Author: Patrick
 */

#include "fixactions_search.h"
#include <vector>
#include <math.h>
#include "option_parser.h"
#include "plugin.h"
#include <cassert>
#include <algorithm>
#include "attack_success_prob_reuse_heuristic.h"
#include "eager_search.h"
#include "budget_dead_end_heuristic.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include "timer.h"

//#define FIX_SEARCH_DEBUG

using namespace std;


FixActionsSearch::FixActionsSearch(const Options &opts) :
    SearchEngine(opts)
{
    search_engine = opts.get<SearchEngine *>("search_engine");

    if (opts.contains("attack_heuristic")) {
        attack_heuristic =
            opts.get<Heuristic *>("attack_heuristic"); // (AttackSuccessProbReuseHeuristic*) (((BudgetDeadEndHeuristic*)opts.get<Heuristic*>("attack_heuristic"))->get_prob_cost_heuristic());
    } else {
        attack_heuristic = NULL;
    }

	attack_budget_factor = opts.get<double>("attack_budget_factor");
	fix_budget_factor = opts.get<double>("fix_budget_factor");
	g_initial_budget = opts.get<int>("initial_attack_budget");
	max_fix_actions_budget = opts.get<int>("initial_fix_budget");

	if (g_initial_budget < UNLTD_BUDGET) {
		g_initial_budget = (int) ((double) g_initial_budget) * attack_budget_factor;
	}
	if(max_fix_actions_budget < UNLTD_BUDGET) {
		max_fix_actions_budget = (int) ((double) max_fix_actions_budget) * fix_budget_factor;
	}

	use_partial_order_reduction = opts.get<bool>("partial_order_reduction");
	check_parent_attack_plan_applicable = opts.get<bool>("check_parent_attack_plan_applicable");
	check_fix_state_already_known = opts.get<bool>("check_fix_state_already_known");
	do_attack_op_dom_pruning = opts.get<bool>("attack_op_dom_pruning");
	use_ids = opts.get<bool>("ids");
	sort_fix_ops_advanced = opts.get<bool>("sort_fix_ops");
	greedy_fix_search = opts.get<bool>("greedy");
}

FixActionsSearch::~FixActionsSearch()
{
}

void FixActionsSearch::initialize()
{
    cout << "Initializing FixActionsSearch..." << endl;

    chrono::high_resolution_clock::time_point t1 =
        chrono::high_resolution_clock::now();

    sort_operators();

    if (fix_operators.size() < 1) {
        // If there are no fix actions, just do one attacker search
        search_engine->search();
        exit(0);
    }

    divide_variables();

    clean_attack_actions();

    create_new_variable_indices();

    check_fix_vars_attacker_preconditioned();

    cout << "fix_variable_domain.size() = " << fix_variable_domain.size() << endl;
    cout << "fix_vars_attacker_preconditioned.size() = " << fix_vars_attacker_preconditioned.size() << endl;

    fix_search_node_infos_attack_plan.set_relevant_variables(
        fix_vars_attacker_preconditioned);

    fix_operators_successor_generator = create_successor_generator(
                                            fix_variable_domain, fix_operators, fix_operators);

    attack_operators_for_fix_vars_successor_generator = create_successor_generator(
                fix_variable_domain,
                attack_operators_with_fix_vars_preconds, attack_operators);

    cout << "attack_operators.size() = " << attack_operators.size() << endl;
    cout << "fix_operators.size() = " << fix_operators.size() << endl;

    compute_commutative_and_dependent_fix_ops_matrices();

    if (use_partial_order_reduction) {
        compute_fix_facts_ops_sets();
    }

    if (do_attack_op_dom_pruning) {
        compute_op_dominance_relation(attack_operators, dominated_attack_op_ids);
    }

    g_all_attack_operators.insert(g_all_attack_operators.begin(),
                                  attack_operators.begin(), attack_operators.end());

    g_operators.clear();
    g_operators.insert(g_operators.begin(),
                       attack_operators.begin(), attack_operators.end());
    g_attack_op_included.resize(attack_operators.size(), true);

    if (sort_fix_ops_stupid && use_partial_order_reduction) {
        delrax_search::DelRaxSearch *delrax_search = (delrax_search::DelRaxSearch *)
                search_engine;
        delrax_search->initialize();
        sortFixActionsByAttackerReward = new SortFixActionsByAttackerReward(
            delrax_search->get_positive_values(),
            delrax_search->get_reward(), fix_operators,
            attack_operators_with_fix_vars_preconds,
            deleting_fix_facts_ops);
    }

    chrono::high_resolution_clock::time_point t2 =
        chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();
    fix_search_initialize_duration = duration;
}

void FixActionsSearch::sort_operators()
{
    cout << "Begin sort_operators()..." << endl;
    int fix_action_op_id = 0;
    int attack_action_op_id = 0;
    for (size_t op_no = 0; op_no < g_operators.size(); op_no++) {
#ifdef FIX_SEARCH_DEBUG
        cout << "Consider op " << op_no << ":" << endl;
        g_operators[op_no].dump();
#endif

		string op_name = g_operators[op_no].get_name();

		size_t whitespace = op_name.find(" ");
		if (whitespace == string::npos) {
			whitespace = op_name.size();
		}
		string everything_before_whitespace = op_name.substr(0, whitespace);
		size_t underscore = everything_before_whitespace.find_last_of("_");
		if (underscore == string::npos) {
			cout << "No cost suffix found! Error in PDDL file?" << endl;
			exit(EXIT_INPUT_ERROR);
		}

		if (op_name.find("attack") == 0) {
			//string prob = everything_before_whitespace.substr(underscore + 1);
			//int success_prob_cost = parse_success_prob_cost(prob);
			// Note that cost and cost2 are swapped here on purpose!
			//g_operators[op_no].set_cost2(g_operators[op_no].get_cost());
			//g_operators[op_no].set_cost(success_prob_cost);
			g_operators[op_no].set_op_id(attack_action_op_id);
			attack_action_op_id++;

			attack_operators.push_back(g_operators[op_no]);

		} else if (op_name.find("fix") == 0) {
			/* We removed the invention cost and id from fix action names
			string invention_cost_and_id_string = everything_before_whitespace.substr(underscore + 1);
			size_t hash = invention_cost_and_id_string.find("#");
			if (hash == string::npos) {
				cout << "No correct invention_cost_and_id_string suffix found! Error in PDDL file?" << endl;
				exit(EXIT_INPUT_ERROR);
			}

			string invention_cost_string = invention_cost_and_id_string.substr(0, hash);
			string id_string = invention_cost_and_id_string.substr(hash + 1);
			int invention_cost = stoi(invention_cost_string);
			int id = stoi(id_string);
			 */

			if (g_operators[op_no].get_cost() > max_fix_action_cost) {
				max_fix_action_cost = g_operators[op_no].get_cost();
			}

			g_operators[op_no].set_cost2(0);
			g_operators[op_no].set_conds_variable_name(fix_variable_name);
			g_operators[op_no].set_effs_variable_name(fix_variable_name);
			g_operators[op_no].set_scheme_id(0);
			g_operators[op_no].set_op_id(fix_action_op_id);
			fix_action_op_id++;

			fix_operators.push_back(g_operators[op_no]);

		} else {
			cout << "No op prefix found! Error in PDDL file?" << endl;
			exit(EXIT_INPUT_ERROR);
		}
	}
}

int FixActionsSearch::parse_success_prob_cost(string prob)
{
    size_t backslash = prob.find("/");
    if (backslash == string::npos) {
        cout << "No correct success probability suffix found! Error in PDDL file?" <<
             endl;
        exit(EXIT_INPUT_ERROR);
    }

    string numerator_string = prob.substr(0, backslash);
    string denominator_string = prob.substr(backslash + 1);
    double numerator = (double) stoi(numerator_string);
    double denominator = (double) stoi(denominator_string);
    return (int)(fabs(log2(numerator / denominator)) * 1000);
}

double FixActionsSearch::prob_cost_to_prob(int prob_cost)
{
    if (prob_cost == ATTACKER_TASK_UNSOLVABLE) {
        return 0.0;
    }
    return pow(2.0, -(((double)prob_cost) / 1000));
}

void FixActionsSearch::divide_variables()
{
    cout << "Begin divide_ariables()..." << endl;
    num_attack_vars = 0;
    attack_vars.assign(g_variable_domain.size(), false);

    for (size_t op_no = 0; op_no < attack_operators.size(); op_no++) {
        const vector<GlobalEffect> &effects = attack_operators[op_no].get_effects();
        for (size_t i = 0; i < effects.size(); i++) {
            int var = effects[i].var;
            if (attack_vars[var]) {
                continue;
            } else {
                attack_vars[var] = true;
                num_attack_vars++;
            }
        }
    }
    num_vars = g_variable_domain.size();
    num_fix_vars = num_vars - num_attack_vars;
}

void FixActionsSearch::clean_attack_actions()
{
    cout << "Begin clean_attack_actions()..." << endl;

    for (size_t op_no = 0; op_no < attack_operators.size(); op_no++) {
        const GlobalOperator &op = attack_operators[op_no];
        const vector<GlobalCondition> &conditions = op.get_preconditions();
        vector<GlobalCondition> fix_preconditions;
        vector<GlobalCondition> attack_preconditions;
        for (size_t i = 0; i < conditions.size(); i++) {
            int var = conditions[i].var;
            if (attack_vars[var]) {
                // This is a precondition on an attack var
                attack_preconditions.push_back(conditions[i]);
            } else {
                // This is a precondition on a fix var
                fix_preconditions.push_back(conditions[i]);
            }
        }

        GlobalOperator op_with_attack_preconds(op.is_axiom(), attack_preconditions,
                                               op.get_effects(), op.get_name(),
                                               op.get_cost(), op.get_cost2(), op.get_op_id(), g_variable_name,
                                               g_variable_name);
        attack_operators[op_no] = op_with_attack_preconds;

        GlobalOperator op_with_fix_preconds(op.is_axiom(), fix_preconditions,
                                            op.get_effects(), op.get_name(),
                                            op.get_cost(), op.get_cost2(), op.get_op_id(), fix_variable_name,
                                            g_variable_name);
        attack_operators_with_fix_vars_preconds.push_back(op_with_fix_preconds);
    }
}

void FixActionsSearch::create_new_variable_indices()
{
    cout << "Begin create_new_variable_indices()..." << endl;

    int curr_attack_var_index = 0;
    int curr_fix_var_index = 0;

    map_var_id_to_new_var_id.resize(num_vars);

    for (int var = 0; var < num_vars; var++) {
        if (attack_vars[var]) {
            // This is an attack var
            map_var_id_to_new_var_id[var] = curr_attack_var_index;
            curr_attack_var_index++;
        } else {
            // This is a fix var
            map_var_id_to_new_var_id[var] = curr_fix_var_index;
            curr_fix_var_index++;
        }
    }

    adjust_var_indices_of_ops(fix_operators);

    adjust_var_indices_of_ops(attack_operators);

    adjust_var_indices_of_ops(attack_operators_with_fix_vars_preconds);

    // Save the fix var stuff locally and clean g_variable_domain, g_variable_name and g_fact_names
    int num_vars_temp = num_vars;
    int var = 0;
    for (int i = 0; i < num_vars_temp; i++) {
        if (!attack_vars[var]) {
            // This is a fix var
            // Save it to local vectors
            fix_variable_domain.push_back(g_variable_domain[i]);
            fix_variable_name.push_back(g_variable_name[i]);
            fix_fact_names.push_back(g_fact_names[i]);
            fix_initial_state_data.push_back(g_initial_state_data[i]);

            // Erase it from vectors
            g_variable_domain.erase(g_variable_domain.begin() + i);
            g_variable_name.erase(g_variable_name.begin() + i);
            g_fact_names.erase(g_fact_names.begin() + i);
            g_initial_state_data.erase(g_initial_state_data.begin() + i);

            // Decrement i and num_vars_temp, because an element was deleted.
            i--;
            num_vars_temp--;
        }
        // Increment var which is the absolute old var id
        var++;
    }

    // Changing indices in g_goal to attack_var indices and ensuring that there is no fix goal variable
    for (size_t i = 0; i < g_goal.size(); i++) {
        int var = g_goal[i].first;
        if (!attack_vars[var]) {
            cout << "There should be no goal defined for a non-attack var! Error in PDDL!"
                 << endl;
            //exit(EXIT_INPUT_ERROR);
        }
        g_goal[i].first = map_var_id_to_new_var_id[g_goal[i].first];
    }

    // Creating two new state_registries, one locally only for fix variables and one globally only for attack variables
    IntPacker *fix_vars_state_packer = new IntPacker(fix_variable_domain);
    fix_vars_state_registry = new StateRegistry(fix_vars_state_packer,
            fix_initial_state_data);
    delete g_state_packer;
    g_state_packer = new IntPacker(g_variable_domain);
    delete g_state_registry;
    g_state_registry = new StateRegistry(g_state_packer, g_initial_state_data);
}

bool cond_comp_func(GlobalCondition cond1, GlobalCondition cond2)
{
    return cond1.var < cond2.var;
}
bool eff_comp_func(GlobalEffect eff1, GlobalEffect eff2)
{
    return eff1.var < eff2.var;
}
void FixActionsSearch::adjust_var_indices_of_ops(vector<GlobalOperator> &ops)
{
    // Adjust indices in preconditions and effects of all operators in ops vector
    for (size_t op_no = 0; op_no < ops.size(); op_no++) {
        vector<GlobalCondition> &conditions = ops[op_no].get_preconditions();
        for (size_t cond_no = 0; cond_no < conditions.size(); cond_no++) {
            conditions[cond_no].var = map_var_id_to_new_var_id[conditions[cond_no].var];
        }

        vector<GlobalEffect> &effects = ops[op_no].get_effects();
        for (size_t eff_no = 0; eff_no < effects.size(); eff_no++) {
            effects[eff_no].var = map_var_id_to_new_var_id[effects[eff_no].var];
        }

        // Sort the conditions and effects by their respective var id
        sort(conditions.begin(), conditions.end(), cond_comp_func);
        sort(effects.begin(), effects.end(), eff_comp_func);
    }
}

void FixActionsSearch::check_fix_vars_attacker_preconditioned()
{
    cout << "Begin check_fix_vars_attacker_preconditioned()..." << endl;

    vector<bool> is_fix_var_attacker_preconditioned(fix_variable_domain.size(),
            false);

    for (size_t op_no = 0; op_no < attack_operators_with_fix_vars_preconds.size();
            op_no++) {
        const vector<GlobalCondition> &conditions =
            attack_operators_with_fix_vars_preconds[op_no].get_preconditions();

        for (size_t cond_no = 0; cond_no < conditions.size(); cond_no++) {
            int var = conditions[cond_no].var;
            is_fix_var_attacker_preconditioned[var] = true;
        }
    }

    for (size_t var = 0; var < fix_variable_domain.size(); var++) {
        if (is_fix_var_attacker_preconditioned[var]) {
            fix_vars_attacker_preconditioned.push_back(var);
        }
    }
}

/**
 * returns a SuccessorGeneratorSwitch based on the preconditions of the ops in pre_cond_ops and entailing the ops from ops vector in the leaves
 */
SuccessorGeneratorSwitch *FixActionsSearch::create_successor_generator(
    const vector<int> &variable_domain,
    const vector<GlobalOperator> &pre_cond_ops, const vector<GlobalOperator> &ops)
{
#ifdef FIX_SEARCH_DEBUG
    cout << "Begin create_successor_generator..." << endl;
#endif

    int root_var_index = 0;

    SuccessorGeneratorSwitch *root_node = new SuccessorGeneratorSwitch(
        root_var_index,
        variable_domain[root_var_index]);

    for (size_t op_no = 0; op_no < pre_cond_ops.size(); op_no++) {
        /*#ifdef FIX_SEARCH_DEBUG
        		cout << "Consider op " << op_no << endl;
        		pre_cond_ops[op_no].dump();
        #endif*/
        vector<GlobalCondition> conditions = pre_cond_ops[op_no].get_preconditions();

        if (conditions.size() == 0) {
            // This op has no preconditions, add it immediately to the root node
            if (root_node->immediate_ops == NULL) {
                root_node->immediate_ops = new SuccessorGeneratorGenerate();
            }
            ((SuccessorGeneratorGenerate *) root_node->immediate_ops)->add_op(&ops[op_no]);
            continue;
        }

        SuccessorGeneratorSwitch *current_node = root_node;
        for (size_t cond_no = 0; cond_no < conditions.size(); cond_no++) {
            int var = conditions[cond_no].var;
            int val = conditions[cond_no].val;
            //cout << "Consider precond with var: " << var << ", val: " << val << endl;

            while (var != current_node->switch_var) {
                if (current_node->default_generator == NULL) {
                    int next_var_index = current_node->switch_var + 1;
                    current_node->default_generator = new SuccessorGeneratorSwitch(next_var_index,
                            variable_domain[next_var_index]);
                }

                current_node = (SuccessorGeneratorSwitch *) current_node->default_generator;
            }

            // Here: var == current_node->switch_var

            int next_var_index = current_node->switch_var + 1;
            if (next_var_index >= (int) variable_domain.size()) {
                if (current_node->generator_for_value[val] == NULL) {
                    current_node->generator_for_value[val] = new SuccessorGeneratorGenerate();
                }
                ((SuccessorGeneratorGenerate *) current_node->generator_for_value[val])->add_op(
                    &ops[op_no]);

            } else {
                if (current_node->generator_for_value[val] == NULL) {
                    current_node->generator_for_value[val] = new SuccessorGeneratorSwitch(
                        next_var_index,
                        variable_domain[next_var_index]);
                }

                current_node = (SuccessorGeneratorSwitch *)
                               current_node->generator_for_value[val];
                if (cond_no == (conditions.size() - 1)) {
                    // This was the last cond.
                    if (current_node->immediate_ops == NULL) {
                        current_node->immediate_ops = new SuccessorGeneratorGenerate();
                    }
                    ((SuccessorGeneratorGenerate *) current_node->immediate_ops)->add_op(
                        &ops[op_no]);
                }
            }
        }
    }

    return root_node;
}

/**
 * For every pair of ops op1 and op2, this method checks whether op1 has a precond on var v on which op2 has an effect and vice versa.
 * If this is the case, op1 and op2 are not commutative. Only if the precond value is different from effect value, op1 and op2 are dependent.
 * If op1 and op2 both effect a var v with different effect values,
 * there are also not commutative and are dependent. Otherwise, they are commutative and not dependet.
 */
void FixActionsSearch::compute_commutative_and_dependent_fix_ops_matrices()
{
    cout << "Begin compute_commutative_and_dependent_fix_ops_matrices()..." << endl;

 /*   cout << "We assume that all fix actions are commutative and not dependent!" <<
         endl;
    {
        vector<bool> val1(fix_operators.size(), true);
        commutative_fix_ops.assign(fix_operators.size(), val1);
        vector<bool> val2(fix_operators.size(), false);
        dependent_fix_ops.assign(fix_operators.size(), val2);
        return;
    }*/


    vector<bool> val(fix_operators.size());
    commutative_fix_ops.assign(fix_operators.size(), val);
    dependent_fix_ops.assign(fix_operators.size(), val);
    for (size_t op_no1 = 0; op_no1 < fix_operators.size(); op_no1++) {
        for (size_t op_no2 = op_no1 + 1; op_no2 < fix_operators.size(); op_no2++) {
            /*#ifdef FIX_SEARCH_DEBUG
            			cout << "Comparing op1 with id " << op_no1 << ":" << endl;
            			fix_operators[op_no1].dump();
            			cout << "to op2 with id " << op_no2 << ":" << endl;
            			fix_operators[op_no2].dump();
            #endif*/

            const vector<GlobalCondition> &conditions1 =
                fix_operators[op_no1].get_preconditions();
            const vector<GlobalCondition> &conditions2 =
                fix_operators[op_no2].get_preconditions();
            const vector<GlobalEffect> &effects1 = fix_operators[op_no1].get_effects();
            const vector<GlobalEffect> &effects2 = fix_operators[op_no2].get_effects();

            bool commutative = true;
            bool dependent = false;

            int i_eff2 = 0;
            for (int i_cond1 = 0; i_cond1 < (int) conditions1.size(); i_cond1++) {
                if (!commutative && dependent) {
                    break;
                }

                int var = conditions1[i_cond1].var;
                int val = conditions1[i_cond1].val;

                while (i_eff2 < ((int) effects2.size() - 1) && effects2[i_eff2].var < var) {
                    i_eff2++;
                }

                if (i_eff2 < (int) effects2.size() && effects2[i_eff2].var == var) {
                    commutative = false;
                    if (val != effects2[i_eff2].val) {
                        dependent = true;
                    }
                }
            }

            int i_eff1 = 0;
            for (int i_cond2 = 0; i_cond2 < (int) conditions2.size(); i_cond2++) {
                if (!commutative && dependent) {
                    break;
                }

                int var = conditions2[i_cond2].var;
                int val = conditions2[i_cond2].val;

                while (i_eff1 < ((int) effects1.size() - 1) && effects1[i_eff1].var < var) {
                    i_eff1++;
                }

                if (i_eff1 < (int) effects1.size() && effects1[i_eff1].var == var) {
                    commutative = false;
                    if (val != effects1[i_eff1].val) {
                        dependent = true;
                    }
                }
            }

            i_eff2 = 0;
            for (i_eff1 = 0; i_eff1 < (int) effects1.size(); i_eff1++) {
                if (!commutative && dependent) {
                    break;
                }

                int var = effects1[i_eff1].var;
                int val = effects1[i_eff1].val;

                while (i_eff2 < ((int) effects2.size() - 1) && effects2[i_eff2].var < var) {
                    i_eff2++;
                }

                if (i_eff2 < (int) effects2.size() && effects2[i_eff2].var == var) {
                    if (val != effects2[i_eff2].val) {
                        commutative = false;
                        dependent = true;
                    }
                }
            }

            /*#ifdef FIX_SEARCH_DEBUG
            		cout << "ops are commutative?: " << commutative << endl;
            		cout << "ops are dependent?: " << dependent << endl;
            #endif*/
            commutative_fix_ops[op_no1][op_no2] = commutative;
            commutative_fix_ops[op_no2][op_no1] = commutative;

            dependent_fix_ops[op_no1][op_no2] = dependent;
            dependent_fix_ops[op_no2][op_no1] = dependent;
        }
    }
}

void FixActionsSearch::compute_op_dominance_relation(const
        vector<GlobalOperator> &ops, vector<vector<int>> &dominated_op_ids)
{
    cout << "Begin compute_op_dominance_relation()..." << endl;
    dominated_op_ids.assign(ops.size(), vector<int>());

    for (size_t op_no1 = 0; op_no1 < ops.size(); op_no1++) {
        for (size_t op_no2 = 0; op_no2 < ops.size(); op_no2++) {
            if (op_no1 == op_no2) {
                continue;
            }

            bool dominated_or_equivalent = true;

            const GlobalOperator &op1 = ops[op_no1];
            const GlobalOperator &op2 = ops[op_no2];

            //cout << "Checking dominacce of op1 with id " << op_no1 << ":" << endl;
            //op1.dump();
            //cout << "to op2 with id " << op_no2 << ":" << endl;
            //op2.dump();

            if (op1.get_cost() > op2.get_cost() || op1.get_cost2() > op2.get_cost2()) {
                continue;
            }

            const vector<GlobalCondition> &conditions1 = op1.get_preconditions();
            const vector<GlobalCondition> &conditions2 = op2.get_preconditions();
            const vector<GlobalEffect> &effects1 = op1.get_effects();
            const vector<GlobalEffect> &effects2 = op2.get_effects();

            if (conditions1.size() > conditions2.size()
                    || effects1.size() != effects2.size()) {
                continue;
            }

            // The effects need to be the same. We already know that effects1.size() == effects2.size()
            for (size_t i_eff = 0; i_eff < effects1.size(); i_eff++) {
                if (effects1[i_eff].var != effects2[i_eff].var
                        || effects1[i_eff].val != effects2[i_eff].val) {
                    dominated_or_equivalent = false;
                    break;
                }
            }

            if (!dominated_or_equivalent) {
                continue;
            }

            // Regarding preconditions, every precond of op1 needs to be a precond of op2
            int i_cond2 = 0;
            for (int i_cond1 = 0; i_cond1 < (int) conditions1.size(); i_cond1++) {
                int var = conditions1[i_cond1].var;
                int val = conditions1[i_cond1].val;

                while (i_cond2 < ((int) conditions2.size() - 1)
                        && conditions2[i_cond2].var < var) {
                    i_cond2++;
                }

                if (i_cond2 >= (int) conditions2.size() || conditions2[i_cond2].var != var
                        || conditions2[i_cond2].val != val) {
                    dominated_or_equivalent = false;
                    break;
                }
            }

            /*#ifdef FIX_SEARCH_DEBUG
            			cout << "op1 dominates op2?: " << dominated_or_equivalent << endl;
            //#endif*/
            if (dominated_or_equivalent) {
                dominated_op_ids[op_no1].push_back(op_no2);
            }
        }
    }
}

void FixActionsSearch::compute_fix_facts_ops_sets()
{
    cout << "Begin compute_fix_facts_ops_sets()..." << endl;

    deleting_fix_facts_ops.resize(num_fix_vars);
    achieving_fix_facts_ops.resize(num_fix_vars);

    for (int var = 0; var < num_fix_vars; var++) {
        int domain_size = fix_variable_domain[var];
        deleting_fix_facts_ops[var].resize(domain_size);
        achieving_fix_facts_ops[var].resize(domain_size);
    }

    for (size_t op_no = 0; op_no < fix_operators.size(); op_no++) {
        const GlobalOperator *op = &fix_operators[op_no];

        const vector<GlobalEffect> &effects = op->get_effects();
        for (size_t eff_no = 0; eff_no < effects.size(); eff_no++) {
            int var = effects[eff_no].var;
            int effect_val = effects[eff_no].val;

            achieving_fix_facts_ops[var][effect_val].push_back(op);

            for (int other_val = 0; other_val < fix_variable_domain[var]; other_val++) {
                if (other_val != effect_val) {
                    deleting_fix_facts_ops[var][other_val].push_back(op);
                }
            }
        }
    }
}

void FixActionsSearch::get_all_dependent_ops(const GlobalOperator *op,
        vector<const GlobalOperator *> &result)
{
    for (size_t other_op_no = 0; other_op_no < dependent_fix_ops.size();
            other_op_no++) {
        if (dependent_fix_ops[op->get_op_id()][other_op_no]) {
            result.push_back(&fix_operators[other_op_no]);
        }
    }
}

void FixActionsSearch::prune_applicable_fix_ops_sss(const GlobalState &state,
        const vector<int> &attack_plan,
        const vector<const GlobalOperator *> &applicable_ops,
        vector<const GlobalOperator *> &result)
{
    unordered_set<const GlobalOperator *> applicable_ops_set(applicable_ops.begin(),
            applicable_ops.end());

    vector<const GlobalOperator *> current_T_s;
    vector<bool> is_in_current_T_s(fix_operators.size(), false);
    // Initialize T_s to the disjunctive action landmark
    // We rely on g_plan containing the currently computed attacker plan
    for (size_t op_no = 0; op_no < attack_plan.size(); op_no++) {
        int op_id = attack_plan[op_no];
        const GlobalOperator *op = &attack_operators_with_fix_vars_preconds[op_id];

        const vector<GlobalCondition> &preconditions = op->get_preconditions();
        for (size_t precond_no = 0; precond_no < preconditions.size(); precond_no++) {
            int precond_var = preconditions[precond_no].var;
            int precond_val = preconditions[precond_no].val;
            const vector< const GlobalOperator *> &deleting_ops =
                deleting_fix_facts_ops[precond_var][precond_val];

            for (size_t del_op_no = 0; del_op_no < deleting_ops.size(); del_op_no++) {
                if (!is_in_current_T_s[deleting_ops[del_op_no]->get_op_id()]) {
                    current_T_s.push_back(deleting_ops[del_op_no]);
                    is_in_current_T_s[deleting_ops[del_op_no]->get_op_id()] = true;
                }
            }
        }
    }

    vector<bool> is_in_result(fix_operators.size(), false);

    for (size_t op_no = 0; op_no < current_T_s.size(); op_no++) {
        const GlobalOperator *op = current_T_s[op_no];
        if (applicable_ops_set.find(op) != applicable_ops_set.end()) {
            // op is in applicable_ops_set
            if (!is_in_result[op->get_op_id()]) {
                is_in_result[op->get_op_id()] = true;
            }

            vector<const GlobalOperator *> dependent_ops;
            get_all_dependent_ops(op, dependent_ops);

            for (size_t dep_op_no = 0; dep_op_no < dependent_ops.size(); dep_op_no++) {
                if (!is_in_current_T_s[dependent_ops[dep_op_no]->get_op_id()]) {
                    current_T_s.push_back(dependent_ops[dep_op_no]);
                    is_in_current_T_s[dependent_ops[dep_op_no]->get_op_id()] = true;
                }
            }
        } else {
            // op not in applicable_ops_set
            // Compute some necessary enabling set for s and op and directly add it to current_T_s
            const vector<GlobalCondition> &preconditions = op->get_preconditions();
            for (size_t precond_no = 0; precond_no < preconditions.size(); precond_no++) {
                int precond_var = preconditions[precond_no].var;
                int precond_val = preconditions[precond_no].val;
                if (state[precond_var] != precond_val) {
                    // We found a precond. fact which is not true in the current state
                    const vector< const GlobalOperator *> &achieving_ops =
                        achieving_fix_facts_ops[precond_var][precond_val];

                    for (size_t achieving_op_no = 0; achieving_op_no < achieving_ops.size();
                            achieving_op_no++) {
                        if (!is_in_current_T_s[achieving_ops[achieving_op_no]->get_op_id()]) {
                            current_T_s.push_back(achieving_ops[achieving_op_no]);
                            is_in_current_T_s[achieving_ops[achieving_op_no]->get_op_id()] = true;
                        }
                    }
                    break;
                }
            }
        }
    }

    for (size_t op_no = 0; op_no < applicable_ops.size(); op_no++) {
        if (is_in_result[applicable_ops[op_no]->get_op_id()]) {
            result.push_back(applicable_ops[op_no]);
        }
    }
}

void FixActionsSearch::prune_dominated_ops(vector<const GlobalOperator *> &ops,
        vector<vector<int>> dominated_op_ids)
{
    vector<bool> marked_for_erase(dominated_op_ids.size(), false);
    vector<int> dominated_by(dominated_op_ids.size(), -1);

    //cout << "attack_ops:" << endl;

    for (size_t op_no = 0; op_no < ops.size(); op_no++) {
        //attack_ops[op_no]->dump();
        if (!marked_for_erase[ops[op_no]->get_op_id()]) {
            const vector<int> dominated_ops = dominated_op_ids[ops[op_no]->get_op_id()];
            for (size_t dom_op_no = 0; dom_op_no < dominated_ops.size(); dom_op_no++) {
                marked_for_erase[dominated_ops[dom_op_no]] = true;
                dominated_by[dominated_ops[dom_op_no]] = ops[op_no]->get_op_id();
            }
        }
    }

    vector<const GlobalOperator *>::iterator it = ops.begin();
    for (; it != ops.end();) {
        if (marked_for_erase[(*it)->get_op_id()]) {
            /*cout << "op: " << endl;
            (*it)->dump();
            cout << "pruned because it was dominated by:" << endl;
            attack_operators[dominated_by[(*it)->get_op_id()]].dump();*/

            it = ops.erase(it);
        } else {
            ++it;
        }
    }
}

bool op_ptr_name_comp(const GlobalOperator *op1, const GlobalOperator *op2)
{
    return op1->get_name() < op2->get_name();
}

string FixActionsSearch::fix_state_to_string(const GlobalState &state) {
	string res = "";
	for (size_t i = 0; i < fix_variable_domain.size(); i++) {
		res += to_string(state[i]);
	}
	return res;
}

string FixActionsSearch::ops_to_string(vector<const GlobalOperator *> &ops) {
	sort(ops.begin(), ops.end(), op_ptr_name_comp);
	string res = "";
	for (size_t i = 0; i < ops.size(); i++) {
		if(i > 0) {
			res += " ";
		}
		res += ops[i]->get_name();
	}
	return res;
}


int FixActionsSearch::compute_pareto_frontier(const GlobalState &state,
        vector<const GlobalOperator *> &fix_ops_sequence, int fix_actions_cost,
        const vector<int> &parent_attack_plan, int parent_attack_plan_cost,
        vector<int> &sleep, bool recurse)
{
    num_recursive_calls++;
#ifdef FIX_SEARCH_DEBUG
    if ((num_recursive_calls % 50) == 0) {
        cout << num_recursive_calls << " num recursive calls until now." << endl;
        dump_pareto_frontier();
    }
#endif

#ifdef FIX_SEARCH_DEBUG
    cout << "in call of compute_pareto_frontier for state: " << endl;
    //state.dump_fdr(fix_variable_domain, fix_variable_name);
    cout << "with id: " << state.get_id().hash() << endl;
    cout << "and current fix actions op_sequence: " << endl;
    for (size_t i = 0; i < fix_ops_sequence.size(); i++) {
        fix_ops_sequence[i]->dump();
    }

    cout << "fix_actions_cost: " << fix_actions_cost << endl;
    cout << "fix_action_costs_for_no_attacker_solution: " <<
         fix_action_costs_for_no_attacker_solution << endl;
#endif
    if (!recurse) {
#ifdef FIX_SEARCH_DEBUG
        cout << "We won't recurse in this execution of compute_pareto_frontier" << endl;
#endif
        num_recursive_calls_for_sorting++;
    }

    bool parent_attack_plan_applicable = false;
    if (check_parent_attack_plan_applicable && parent_attack_plan.size() > 0) {
        /*cout << "parent attack plan: " << endl;
        for (size_t op_no = 0; op_no < parent_attack_plan.size(); op_no++) {
        	attack_operators_with_fix_vars_preconds[parent_attack_plan[op_no]].dump();
        }*/

        parent_attack_plan_applicable = true;
        // Check whether parent_attack_plan is still applicable in current fix-state
        for (size_t op_no = 0; op_no < parent_attack_plan.size(); op_no++) {
            if (!attack_operators_with_fix_vars_preconds[parent_attack_plan[op_no]].is_applicable(
                        state)) {
                parent_attack_plan_applicable = false;
                break;
            }
        }
    }

    vector<int> attack_plan;

    AttackSearchSpace *attack_heuristic_search_space = NULL;
    bool free_attack_heuristic_per_state_info = false;

    int attack_plan_cost = ATTACKER_TASK_UNSOLVABLE;
    FixSearchInfoAttackPlan &info_attack_plan =
        fix_search_node_infos_attack_plan[state];
    FixSearchInfoFixSequence &info_fix_sequence =
        fix_search_node_infos_fix_sequence[state];

    if ((check_fix_state_already_known
            && info_attack_plan.attack_plan_prob_cost != -1)
            || parent_attack_plan_applicable) {
        if (check_fix_state_already_known
                && info_attack_plan.attack_plan_prob_cost != -1) {
            attack_plan_cost = info_attack_plan.attack_plan_prob_cost;
            attack_plan = info_attack_plan.attack_plan;
            spared_attacker_searches_because_fix_state_already_seen++;

#ifdef FIX_SEARCH_DEBUG
            cout << "Attack prob cost for this state is already known in PerStateInformation: "
                 << attack_plan_cost << endl;
#endif
            if (info_fix_sequence.fix_actions_cost != -1
                    && fix_actions_cost > info_fix_sequence.fix_actions_cost
					&& info_fix_sequence.already_in_frontier) {
#ifdef FIX_SEARCH_DEBUG
                cout << "Current fix action sequence is more expensive than already known sequence... don't make further recursive calls. "
                     << endl;
#endif
                num_fix_op_paths++;
                return attack_plan_cost;
            } else {
            	info_fix_sequence.fix_actions_cost = fix_actions_cost;
            }
        } else {
            attack_plan_cost = parent_attack_plan_cost;
            if (check_fix_state_already_known) {
                info_attack_plan.attack_plan_prob_cost = attack_plan_cost;
                info_attack_plan.attack_plan = parent_attack_plan;
                info_fix_sequence.fix_actions_cost = fix_actions_cost;
            }
            spared_attacker_searches_because_parent_plan_applicable++;
#ifdef FIX_SEARCH_DEBUG
            cout << "Attack prob cost for this state is already known from parent_attack_plan: "
                 << attack_plan_cost << endl;
#endif
        }

        if (attack_heuristic != NULL) {
            attack_heuristic_search_space =
                attack_heuristic->get_curr_attack_search_space();
        }
    } else {
        num_attacker_searches++;
        vector<const GlobalOperator *> applicable_attack_operators;
        attack_operators_for_fix_vars_successor_generator->generate_applicable_ops(
            state, applicable_attack_operators);

        if (do_attack_op_dom_pruning) {
            prune_dominated_ops(applicable_attack_operators, dominated_attack_op_ids);
        }

        g_operators.clear();
        g_attack_op_included.assign(attack_operators.size(), false);
        //cout << "g_operators: " << endl;
        for (size_t op_no = 0; op_no < applicable_attack_operators.size(); op_no++) {
            g_operators.push_back(*applicable_attack_operators[op_no]);
            g_attack_op_included[applicable_attack_operators[op_no]->get_op_id()] = true;
            //all_attack_operators[op_no]->dump();
        }

        chrono::high_resolution_clock::time_point tt1 =
            chrono::high_resolution_clock::now();

        delete g_successor_generator;
        g_successor_generator = create_successor_generator(g_variable_domain, g_operators, g_operators);
        //g_successor_generator->dump();
        //cout << "Attacker dump everything: " << endl;
        //dump_everything();

        search_engine->reset();
        if (attack_heuristic != NULL) {
            attack_heuristic->reset();
        }
        chrono::high_resolution_clock::time_point tt2 =
            chrono::high_resolution_clock::now();
        auto duration2 = chrono::duration_cast<chrono::milliseconds>(tt2 - tt1).count();
        reset_and_initialize_duration_sum += duration2;

        // Call search, but make sure that no stuff is written to cout when we are not debugging
#ifndef FIX_SEARCH_DEBUG
        streambuf *old = cout.rdbuf(); // <-- save
        stringstream ss;
        cout.rdbuf(ss.rdbuf());        // <-- redirect
#endif
        chrono::high_resolution_clock::time_point t1 =
            chrono::high_resolution_clock::now();
        search_engine->search();       // <-- call
        chrono::high_resolution_clock::time_point t2 =
            chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
        attack_search_duration_sum += duration;

#ifndef FIX_SEARCH_DEBUG
        cout.rdbuf(old);   			// <-- restore
#endif

        SearchSpace *search_space = search_engine->get_search_space();
        all_attacker_states += search_space->get_num_search_node_infos();

		if (search_engine->found_solution()) {
			search_engine->save_plan_if_necessary();
			attack_plan_cost = search_engine->calculate_plan_cost();
#ifdef FIX_SEARCH_DEBUG
            cout << "Attack attack_plan cost is " << attack_plan_cost << endl;
#endif

            if (attack_heuristic != NULL) {
                attack_heuristic_search_space = new AttackSearchSpace();
                free_attack_heuristic_per_state_info = true;

                OpenList<pair<StateID, int>> *open_list = ((EagerSearch *)
                                          search_engine)->get_open_list();
                const GlobalState *goal_state = search_engine->get_goal_state();
                const int goal_state_budget = search_engine->get_goal_state_budget();
                attack_heuristic->reinitialize(attack_heuristic_search_space, search_space,
                                               open_list, *goal_state,
                                               goal_state_budget);
            }
        } else {
#ifdef FIX_SEARCH_DEBUG
            cout << "Attacker task was not solvable!" << endl;
#endif
            attack_plan_cost = ATTACKER_TASK_UNSOLVABLE;
        }
        if (check_fix_state_already_known) {
            info_attack_plan.attack_plan_prob_cost = attack_plan_cost;
            info_fix_sequence.fix_actions_cost = fix_actions_cost;
        }
    }

    // Copy found g_plan to local vector iff we even performed a successful search
    if (!parent_attack_plan_applicable && attack_plan.empty()
            && attack_plan_cost != ATTACKER_TASK_UNSOLVABLE) {
        for (size_t op_no = 0; op_no < g_plan.size(); op_no++) {
            attack_plan.push_back(g_plan[op_no]->get_op_id());
        }
        if (check_fix_state_already_known) {
            info_attack_plan.attack_plan = attack_plan;

            /*vector<const GlobalOperator *> ops(attack_plan.size());
            for (size_t op_no = 0; op_no < attack_plan.size(); op_no++) {
            	ops[op_no] = &attack_operators[attack_plan[op_no]];
            }
            cerr << fix_state_to_string(state) << ": " << ops_to_string(ops) << endl;*/

        }
    }

    if (!recurse) {
        return attack_plan_cost;
    }

	if (!info_fix_sequence.already_in_frontier) {
		vector<vector<const GlobalOperator *>> temp_list_op_sequences;
		temp_list_op_sequences.push_back(fix_ops_sequence);
		triple<int, int, vector<vector<const GlobalOperator *>>> curr_node = make_tuple(
				fix_actions_cost, attack_plan_cost, temp_list_op_sequences);
		add_node_to_pareto_frontier(curr_node);
		info_fix_sequence.already_in_frontier = true;
	}

    if (attack_plan_cost == ATTACKER_TASK_UNSOLVABLE) {
        // Return, if attacker task was not solvable
        num_fix_op_paths++;
        return attack_plan_cost;
    }


    vector<const GlobalOperator *> applicable_ops;
    fix_operators_successor_generator->generate_applicable_ops(state,
            applicable_ops);


    //sort(applicable_ops.begin(), applicable_ops.end(), op_ptr_name_comp);

    /*cout << "applicable ops: " << endl;
    for (size_t op_no = 0; op_no < applicable_ops.size(); op_no++) {
    	applicable_ops[op_no]->dump();
    }*/

    vector<const GlobalOperator *> applicable_ops_after_pruning;
    if (use_partial_order_reduction) {
        prune_applicable_fix_ops_sss(state,
                                     parent_attack_plan_applicable ? parent_attack_plan : attack_plan,
                                     applicable_ops, applicable_ops_after_pruning);
    } else {
        applicable_ops_after_pruning.swap(applicable_ops);
    }

    /*cout << "applicable ops after pruning: " << endl;
    for (size_t op_no = 0; op_no < applicable_ops_after_pruning.size(); op_no++) {
    	applicable_ops_after_pruning[op_no]->dump();
    }*/

    if (sortFixActionsByAttackerReward != NULL) {
        cout << "Sort fix ops!" << endl;
        sortFixActionsByAttackerReward->sort_fix_ops(applicable_ops);
        cout << "After sorting fix ops" << endl;
    }

    if (sort_fix_ops_advanced) {
        vector<int> recursive_attacker_costs(fix_operators.size(), NO_ATTACKER_COST);

        iterate_applicable_ops(applicable_ops_after_pruning, state,
                               parent_attack_plan_applicable ? parent_attack_plan : attack_plan,
                               attack_plan_cost, fix_ops_sequence, sleep, attack_heuristic_search_space, false,
                               recursive_attacker_costs);

        struct myComp {
            const vector<int> &recursive_attacker_costs;
            myComp(const vector<int> &_recursive_attacker_costs):
                recursive_attacker_costs(_recursive_attacker_costs) {}
            bool operator()(const GlobalOperator *op1, const GlobalOperator *op2)
            {
                if (recursive_attacker_costs[op1->get_op_id()] ==
                        recursive_attacker_costs[op2->get_op_id()]) {
                    return op1->get_cost() < op2->get_cost();
                } else {
                    return recursive_attacker_costs[op1->get_op_id()] >
                           recursive_attacker_costs[op2->get_op_id()];
                }
            }
        } myobject(recursive_attacker_costs);

        sort(applicable_ops_after_pruning.begin(), applicable_ops_after_pruning.end(),
             myobject);
    }

    vector<int> dummy;

    // cerr << fix_state_to_string(state) << ": " << ops_to_string(applicable_ops_after_pruning) << endl;

    iterate_applicable_ops(applicable_ops_after_pruning, state,
                           parent_attack_plan_applicable ? parent_attack_plan : attack_plan,
                           attack_plan_cost, fix_ops_sequence, sleep, attack_heuristic_search_space, true,
                           dummy);

    if (free_attack_heuristic_per_state_info) {
        delete attack_heuristic_search_space;
    }

    return attack_plan_cost;
}

void FixActionsSearch::iterate_applicable_ops(const
        vector<const GlobalOperator *> &applicable_ops,
        const GlobalState &state, const vector<int> &attack_plan, int attack_plan_cost,
        vector<const GlobalOperator *> &fix_ops_sequence,
        vector<int> &sleep, AttackSearchSpace *attack_heuristic_search_space,
        bool recurse, vector<int> &recursive_attacker_costs)
{
    /*cout << "applicable ops after pruning: " << endl;
     for (size_t op_no = 0; op_no < applicable_ops.size(); op_no++) {
     applicable_ops[op_no]->dump();
     }*/
    bool at_least_one_recursion = false;

    vector<const GlobalOperator *> actually_recursed_ops;

    for (size_t op_no = 0; op_no < applicable_ops.size(); op_no++) {

    	if(sort_fix_ops_advanced && greedy_fix_search && recurse && at_least_one_recursion) {
    		// Greedily break the loop here after the first actual recursion
    		break;
    	}

        const GlobalOperator *op = applicable_ops[op_no];
        if (find(fix_ops_sequence.begin(), fix_ops_sequence.end(),
                 applicable_ops[op_no])
                != fix_ops_sequence.end()) {
            // Continue, if op is already in sequence
            continue;
        }
        if (sleep[op->get_op_id()] != 0) {
            // Continue, if op is in sleep set
            continue;
        }
        fix_ops_sequence.push_back(op);
        int new_fix_actions_cost = calculate_fix_actions_plan_cost(fix_ops_sequence);
        if (new_fix_actions_cost > curr_fix_actions_budget) {
            // Continue, if adding this op exceeds the budget
            fix_ops_sequence.pop_back();
            returned_somewhere_bc_of_budget = true;
            continue;
        }
        if (new_fix_actions_cost > fix_action_costs_for_no_attacker_solution) {
            //cout
            //       << "Do not continue with this op, because the new fix_action_cost is already greater than fix_action_costs_for_no_attacker_solution"
            //        << endl;
            fix_ops_sequence.pop_back();
            continue;
        }

        actually_recursed_ops.push_back(op);

        // Add all ops before op_no in applicable_ops to sleep set if they are commutative
        int op_id = op->get_op_id();
        for (size_t op_no2 = 0; op_no2 < op_no; op_no2++) {
            int op2_id = applicable_ops[op_no2]->get_op_id();
            if (commutative_fix_ops[op_id][op2_id]) {
                sleep[op2_id]++;
            }
        }
        const GlobalState &next_state = fix_vars_state_registry->get_successor_state(
                                            state, *op);
        if (attack_heuristic != NULL) {
            attack_heuristic->set_curr_attack_search_space(attack_heuristic_search_space);
        }
        at_least_one_recursion = true;
        int attacker_cost = compute_pareto_frontier(next_state, fix_ops_sequence,
                            new_fix_actions_cost,
                            attack_plan, attack_plan_cost, sleep, recurse);
        if (!recurse) {
            recursive_attacker_costs[op->get_op_id()] = attacker_cost;
        }
        // Remove all ops before op_no in applicable_ops from sleep set if they are commutative
        for (size_t op_no2 = 0; op_no2 < op_no; op_no2++) {
            int op2_id = applicable_ops[op_no2]->get_op_id();
            if (commutative_fix_ops[op_id][op2_id]) {
                sleep[op2_id]--;
            }
        }
        fix_ops_sequence.pop_back();
    }
    if (!at_least_one_recursion) {
        num_fix_op_paths++;
    }
     if(recurse && at_least_one_recursion) {
        cerr << fix_state_to_string(state) << ": " << ops_to_string(actually_recursed_ops) << endl;
    }
}

bool pareto_node_comp_func(const
                           triple<int, int, vector<vector<const GlobalOperator *>>> &node1,
                           const triple<int, int, vector<vector<const GlobalOperator *>>> &node2)
{
    return get<0>(node1) < get<0>(node2);
}

void FixActionsSearch::add_node_to_pareto_frontier(
    triple<int, int, vector<vector<const GlobalOperator *>>> &node)
{

    // First check whether attack_prob_costs == Intmax and fix_actions_cost < fix_action_costs_for_no_attacker_solution
    if (get<1>(node) == ATTACKER_TASK_UNSOLVABLE) {
        if (get<0>(node) < fix_action_costs_for_no_attacker_solution) {
            fix_action_costs_for_no_attacker_solution = get<0>(node);
        }
    }

    if (pareto_frontier.size() == 0) {
#ifdef FIX_SEARCH_DEBUG
        cout << "added node with fix cost: " << get<0>(node) << " and attack cost: " <<
             get<1>(node) << " to frontier" << endl;
#endif
        pareto_frontier.push_back(node);
        return;
    }

    auto it = lower_bound(pareto_frontier.begin(), pareto_frontier.end(), node,
                          pareto_node_comp_func);

    if (it == pareto_frontier.end()) {
        // Check whether node is dominated by last element
        it--;
        if (get<1>(*it) < get<1>(node)) {
#ifdef FIX_SEARCH_DEBUG
            cout << "added node with fix cost: " << get<0>(node) << " and attack cost: " <<
                 get<1>(node) << " to frontier" << endl;
#endif
            pareto_frontier.push_back(node);
        }
        return;
    }

    if (it != pareto_frontier.begin()) {
        if (get<1>(*(it - 1)) >= get<1>(node)) {
            // The new node is dominated by existing node with fewer fix action costs
            return;
        }
    }


    if (get<0>(*it) == get<0>(node)) {
        if (get<1>(*it) < get<1>(node)) {
            it = pareto_frontier.erase(it);
            it = pareto_frontier.insert(it, node);
#ifdef FIX_SEARCH_DEBUG
            cout << "added node with fix cost: " << get<0>(node) << " and attack cost: " <<
                 get<1>(node) << " to frontier" << endl;
#endif
            it++;
        } else if (get<1>(*it) == get<1>(node)) {
            if (get<2>(node)[0].size() == 0) {
                // Do not add the empty fix ops sequence again
                // Assert that the the empty fix ops sequence is the only element her in the frontier
                assert(get<2>(*it).size() == 1 && get<2>(*it)[0].size() == 0);
                return;
            }
#ifdef FIX_SEARCH_DEBUG
            cout << "added additional fix action sequence to node with with fix cost: " <<
                 get<0>(node) << " and attack cost: " << get<1>(node) << endl;
#endif
            get<2>(*it).push_back(get<2>(node)[0]);
            return;
        } else {
            return;
        }
    } else {
        it = pareto_frontier.insert(it, node);
        it++;
#ifdef FIX_SEARCH_DEBUG
        cout << "added node with fix cost: " << get<0>(node) << " and attack cost: " <<
             get<1>(node) << " to frontier" << endl;
#endif
    }

    while (it != pareto_frontier.end() && get<1>(*it) <= get<1>(node)) {
        it = pareto_frontier.erase(it);
    }

}

void FixActionsSearch::dump_op_sequence(const vector<const GlobalOperator *>
                                        &op_sequence, std::ostringstream &json)
{

	json << "  [";
    if (op_sequence.size() < 1) {
        cout << "\t\t\t <empty sequence>" << endl;
        json << "]";
        return;
    }

    for (size_t i = 0; i < op_sequence.size(); ++i) {
    	json << (i > 0 ? ", " : "") << "\"" << op_sequence[i]->get_name() << "\"";
        cout << "\t\t\t " << op_sequence[i]->get_name() << endl;
    }
    json << "]";
}

void FixActionsSearch::dump_op_sequence_sequence(const
        vector<vector<const GlobalOperator *>> &op_sequence_sequence, std::ostringstream &json)
{
    for (size_t i = 0; i < op_sequence_sequence.size(); ++i) {
        if (i > 0)  {
            json << ",\n";
        }
        cout << "\t\t sequence " << i << ":" << endl;
        dump_op_sequence(op_sequence_sequence[i], json);
    }
}


//void FixActionsSearch::dump_pareto_frontier_node(triple<int, int, vector<vector<const GlobalOperator*>>> &node) {
//	cout << "\t fix ops costs: " << get<0>(node) << /* ", attack prob: " <<  setprecision(3) << prob_cost_to_prob(get<1>(node)) */ ", attack cost: " << get<1>(node) << ", sequences: " << endl;
//	dump_op_sequence_sequence(get<2>(node));
//}

void FixActionsSearch::dump_pareto_frontier_node(
    triple<int, int, vector<vector<const GlobalOperator *>>> &node, std::ostringstream &json)
{
    cout << "\t fix ops costs: " << get<0>(node) << ", attacker cost: " <<
             get<1>(node) << ", sequences: " << endl;
    json << "{"
         << "\"attacker cost\": " << abs(get<1>(node))
         << ", \"defender cost\": " << get<0>(node)
         << ", \"sequences\": [";
    dump_op_sequence_sequence(get<2>(node), json);
    json << "]}";
}

void FixActionsSearch::dump_pareto_frontier()
{
    std::ostringstream json;
    json << "[";
    cout << "Resulting Pareto-frontier: " << endl;
    for (size_t i = 0; i < pareto_frontier.size(); ++i) {
    	if(i > 0) {
    		json << ",\n";
    	}
        dump_pareto_frontier_node(pareto_frontier[i], json);
    }
    cout << "END Pareto-frontier" << endl;

    json << "]";
    std::ofstream out;
    out.open("pareto_frontier.json");
    out << json.str();
    out.close();
}

SearchStatus FixActionsSearch::step() {
	cout << "Starting fix-actions IDS..." << endl;

	if(use_ids) {
		curr_fix_actions_budget = max(2, max_fix_action_cost);
	} else {
		curr_fix_actions_budget = UNLTD_BUDGET;
	}

	chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

	while (true) {
		cout << "(Re)starting search with fix action budget: " << curr_fix_actions_budget << endl;

		//pareto_frontier.clear();

		vector<const GlobalOperator *> op_sequnce;
		vector<int> parent_attack_plan;
		vector<int> sleep(fix_operators.size(), 0);
		returned_somewhere_bc_of_budget = false;

		compute_pareto_frontier(fix_vars_state_registry->get_initial_state(), op_sequnce, 0, parent_attack_plan, 0, sleep, true);

		if(get<1>(pareto_frontier[pareto_frontier.size() -1 ]) == numeric_limits<int>::max()) {
			break;
		}

		if(!returned_somewhere_bc_of_budget) {
			break;
			cout << "Bla" << endl;
		}

		if(curr_fix_actions_budget >= max_fix_actions_budget) {
			break;
		}

		int new_fix_actions_budget = curr_fix_actions_budget * ids_fix_budget_factor;
		if (new_fix_actions_budget < 0) {
			new_fix_actions_budget = max_fix_actions_budget;
		}

		curr_fix_actions_budget = min(new_fix_actions_budget , max_fix_actions_budget);
	}

    chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>( t2 - t1 ).count();

    cout << "total time: " << g_timer << endl;
    cout << "FixSearch initialize took: " << fix_search_initialize_duration << "ms"
         << endl;
    cout << "Complete Fixsearch took: " << duration << "ms" << endl;
    cout << "Search in Attacker Statespace took " << (attack_search_duration_sum/1000) <<
         "ms" << endl;
    cout << "Search in Fixactions Statespace took " << (duration -
            (attack_search_duration_sum/1000)) << "ms" << endl;
    cout << "reset_and_initialize_duration_sum: " <<
         reset_and_initialize_duration_sum << "ms" << endl;
    cout << "They were in total " << num_recursive_calls <<
         " calls to compute_pareto_frontier." << endl;
    cout << "thereof because of sorting fix actions: " <<
         num_recursive_calls_for_sorting << endl;
    cout << "and " << (num_recursive_calls - num_recursive_calls_for_sorting) <<
         " \"real\" calls" << endl;
    cout << "They were " << num_attacker_searches <<
         " searches in Attacker Statespace" << endl;
    cout << "We spared " << (spared_attacker_searches_because_fix_state_already_seen
                             - (num_recursive_calls - num_recursive_calls_for_sorting) + 1)
         << " attacker searches, because the fix state was already known" << endl;
    cout << "We spared " << spared_attacker_searches_because_parent_plan_applicable
         << " attacker searches, because the fix parent state attack plan was still applicable"
         << endl;
    // cout << "Attacker Searchspace had " << (all_attacker_states / num_attacker_searches) << " states on average" << endl;
    // cout << "Attacker Searchspaces accumulated " << g_state_registry->size() << " states in state_registry" << endl;
    // cout << "Num fix action paths: " << num_fix_op_paths << endl; TODO This is currently not computed correctly
    dump_pareto_frontier();
    exit(EXIT_CRITICAL_ERROR);
    return IN_PROGRESS;
}

void FixActionsSearch::add_options_to_parser(OptionParser &parser)
{
    SearchEngine::add_options_to_parser(parser);
}

SearchEngine *_parse(OptionParser &parser)
{
    FixActionsSearch::add_options_to_parser(parser);
    parser.add_option<SearchEngine *>("search_engine");
    parser.add_option<Heuristic *>("attack_heuristic",
                                   "The heuristic used for search in AttackerStateSpace", "", OptionFlags(false));
    parser.add_option<int>("initial_attack_budget", "The initial attacker Budget",
                           "2147483647");
    parser.add_option<int>("initial_fix_budget", "The initial fix actions Budget",
                           "2147483647");
    parser.add_option<double>("attack_budget_factor",
                              "The factor to multiply with attack actions budget", "1.0");
    parser.add_option<double>("fix_budget_factor",
                              "The factor to multiply with fix actions budget", "1.0");
    parser.add_option<bool>("partial_order_reduction",
                            "use partial order reduction for fix ops", "true");
    parser.add_option<bool>("check_parent_attack_plan_applicable",
                            "always check whether the attacker plan of the parent fix state is still applicable",
                            "true");
    parser.add_option<bool>("check_fix_state_already_known",
                            "always check whether the current fix state is already known and spare search in attacker statespace",
                            "true");
    parser.add_option<bool>("attack_op_dom_pruning",
                            "use the attack operator dominance pruning", "true");
    parser.add_option<bool>("sort_fix_ops",
                            "When expanding fix state successors, first compute attacker cost in all successor states and then recurse in descending cost order.",
                            "true");
    parser.add_option<bool>("ids", "use iterative deepening search", "true");
    parser.add_option<bool>("greedy", "Only makes sense in combination with sort_fix_ops=true. Basically only greedily recurse with best op w.r.t. to fix-op sorting and do not consider the others ", "false");
    Options opts = parser.parse();
    if (!parser.dry_run()) {
        return new FixActionsSearch(opts);
    }
    return NULL;
}

Plugin<SearchEngine> _plugin("fixsearch", _parse);
