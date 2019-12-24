#include "stackelberg_task.h"

#include <iostream>

using namespace std;

namespace stackelberg {


    void StackelbergTask::sort_operators()
    {
        cout << "Begin sort_operators()..." << endl;
        int fix_action_op_id = 0;
        int follower_action_op_id = 0;
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
                // Comment in the following lines for parsing attacker probability from action name
		/*	string prob = everything_before_whitespace.substr(underscore + 1);
			int success_prob_cost = parse_success_prob_cost(prob);
			if (success_prob_cost == -1) {
                        g_operators[op_no].set_cost2(g_operators[op_no].get_cost());
			} else {
                        // Note that cost and cost2 are swapped here on purpose!
                        g_operators[op_no].set_cost2(g_operators[op_no].get_cost());
                        g_operators[op_no].set_cost(success_prob_cost);
			}
                */
                g_operators[op_no].set_op_id(follower_action_op_id);
                follower_action_op_id++;

                follower_operators.push_back(g_operators[op_no]);

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
                exit_with(EXIT_INPUT_ERROR);
            }
	}
    }



    
    double StackelbergTask::prob_cost_to_prob(int prob_cost)
    {
        if (prob_cost == ATTACKER_TASK_UNSOLVABLE) {
            return 0.0;
        }
        return pow(2.0, -(((double)prob_cost) / 1000));
    }

    void StackelbergTask::divide_variables()
    {
        cout << "Begin divide_ariables()..." << endl;
        num_vars = g_variable_domain.size();

        num_follower_vars = 0;
        follower_vars.assign(g_variable_domain.size(), false);

        num_fix_vars = 0;
        fix_vars.assign(g_variable_domain.size(), false);

        for (size_t op_no = 0; op_no < follower_operators.size(); op_no++) {
            const vector<GlobalEffect> &effects = follower_operators[op_no].get_effects();
            for (size_t i = 0; i < effects.size(); i++) {
                int var = effects[i].var;
                if (follower_vars[var]) {
                    continue;
                } else {
                    follower_vars[var] = true;
                    follower_vars_indizes.push_back(var);
                    num_follower_vars++;
                }
            }
        }

        for (size_t op_no = 0; op_no < fix_operators.size(); op_no++) {
            const vector<GlobalEffect> &effects = fix_operators[op_no].get_effects();
            for (size_t i = 0; i < effects.size(); i++) {
                int var = effects[i].var;
                if (fix_vars[var]) {
                    continue;
                } else {
                    fix_vars[var] = true;
                    fix_vars_indizes.push_back(var);
                    num_fix_vars++;
                }
            }

            const vector<GlobalCondition> &preconditions = fix_operators[op_no].get_preconditions();
            for (size_t i = 0; i < preconditions.size(); i++) {
                int var = preconditions[i].var;
                if (fix_vars[var]) {
                    continue;
                } else {
                    fix_vars[var] = true;
                    fix_vars_indizes.push_back(var);
                    num_fix_vars++;
                }
            }
        }

    }

    bool cond_comp_func(GlobalCondition cond1, GlobalCondition cond2)
    {
        return cond1.var < cond2.var;
    }
    bool eff_comp_func(GlobalEffect eff1, GlobalEffect eff2)
    {
        return eff1.var < eff2.var;
    }

    void StackelbergTask::clean_follower_actions()
    {
        cout << "Begin clean_follower_actions()..." << endl;

        for (size_t op_no = 0; op_no < follower_operators.size(); op_no++) {
            const GlobalOperator &op = follower_operators[op_no];
            const vector<GlobalCondition> &conditions = op.get_preconditions();
            vector<GlobalCondition> fix_preconditions;
            vector<GlobalCondition> follower_preconditions;
            for (size_t i = 0; i < conditions.size(); i++) {
                int var = conditions[i].var;
                if (follower_vars[var]) {
                    // This is a precondition on an attack var
                    follower_preconditions.push_back(conditions[i]);
                }
                if (fix_vars[var]) {
                    // This is a precondition on a fix var
                    fix_preconditions.push_back(conditions[i]);
                }
            }

            /* FIXME Because of REMOVED DIVIDING VARIABLES, we added this: */
            follower_operators_with_all_preconds.push_back(op);
            vector<GlobalCondition> &conditions2 = follower_operators_with_all_preconds[op_no].get_preconditions();
            vector<GlobalEffect> &effects = follower_operators_with_all_preconds[op_no].get_effects();
            // Sort the conditions and effects by their respective var id
            sort(conditions2.begin(), conditions2.end(), cond_comp_func);
            sort(effects.begin(), effects.end(), eff_comp_func);


            GlobalOperator op_with_follower_preconds(op.is_axiom(), follower_preconditions,
                                                     op.get_effects(), op.get_name(),
                                                     op.get_cost(), op.get_cost2(), op.get_op_id(), g_variable_name,
                                                     g_variable_name);
            follower_operators[op_no] = op_with_follower_preconds;

            GlobalOperator op_with_fix_preconds(op.is_axiom(), fix_preconditions,
                                                op.get_effects(), op.get_name(),
                                                op.get_cost(), op.get_cost2(), op.get_op_id(), fix_variable_name,
                                                g_variable_name);
            follower_operators_with_fix_vars_preconds.push_back(op_with_fix_preconds);


        }
    }

    void StackelbergTask::create_new_variable_indices()
    {
        cout << "Begin create_new_variable_indices()..." << endl;

        int curr_follower_var_index = 0;
        int curr_fix_var_index = 0;

        map_var_id_to_new_follower_var_id.resize(num_vars);
        map_var_id_to_new_fix_var_id.resize(num_vars);
        map_fix_var_id_to_orig_var_id.resize(num_fix_vars);
        map_follower_var_id_to_orig_var_id.resize(num_follower_vars);

        for (int var = 0; var < num_vars; var++) {
            if (follower_vars[var]) {
                // This is an attack var
                map_var_id_to_new_follower_var_id[var] = curr_follower_var_index;
                map_follower_var_id_to_orig_var_id[curr_follower_var_index] = var;
                curr_follower_var_index++;
            }
            if (fix_vars[var]) {
                // This is a fix var
                map_var_id_to_new_fix_var_id[var] = curr_fix_var_index;
                map_fix_var_id_to_orig_var_id[curr_fix_var_index] = var;
                curr_fix_var_index++;
            }
        }

        adjust_var_indices_of_ops(fix_operators, map_var_id_to_new_fix_var_id, map_var_id_to_new_fix_var_id);

        adjust_var_indices_of_ops(follower_operators, map_var_id_to_new_follower_var_id, map_var_id_to_new_follower_var_id);

        adjust_var_indices_of_ops(follower_operators_with_fix_vars_preconds, map_var_id_to_new_fix_var_id, map_var_id_to_new_follower_var_id);

        // Save the fix var stuff locally and clean g_variable_domain, g_variable_name and g_fact_names
        int num_vars_temp = num_vars;
        int var = 0;
        for (int i = 0; i < num_vars_temp; i++) {
            if (fix_vars[var]) {
                // This is a fix var
                // Save it to local vectors
                fix_variable_domain.push_back(g_variable_domain[i]);
                fix_variable_name.push_back(g_variable_name[i]);
                fix_fact_names.push_back(g_fact_names[i]);
                fix_initial_state_data.push_back(g_initial_state_data[i]);

                /* FIXME REMOVED DIVIDING VARIABLES, so we don't remove the fix actions from the global stuff
                // Erase it from vectors
                g_variable_domain.erase(g_variable_domain.begin() + i);
                g_variable_name.erase(g_variable_name.begin() + i);
                g_fact_names.erase(g_fact_names.begin() + i);
                g_initial_state_data.erase(g_initial_state_data.begin() + i);

                // Decrement i and num_vars_temp, because an element was deleted.
                i--;
                num_vars_temp--;
                */
            }
            // Increment var which is the absolute old var id
            var++;
        }

        // Changing indices in g_goal to follower_var indices and ensuring that there is no fix goal variable
        for (size_t i = 0; i < g_goal.size(); i++) {
            int var = g_goal[i].first;
            if (!follower_vars[var]) {
                cout << "There should be no goal defined for a non-attack var! Error in PDDL!"
                     << endl;
                //exit(EXIT_INPUT_ERROR);
            }
            /* FIXME REMOVED DIVIDING VARIABLES
               g_goal[i].first = map_var_id_to_new_var_id[g_goal[i].first];
            */
        }

        // Creating two new state_registries, one locally only for fix variables and one globally only for attack variables
        IntPacker *fix_vars_state_packer = new IntPacker(fix_variable_domain);
        fix_vars_state_registry = new StateRegistry(fix_vars_state_packer,
                                                    fix_initial_state_data);

        /* FIXME REMOVED DIVIDING VARIABLES
           delete g_state_packer;
           g_state_packer = new IntPacker(g_variable_domain);
           delete g_state_registry;
           g_state_registry = new StateRegistry(g_state_packer, g_initial_state_data);
        */
    }

    void StackelbergTask::adjust_var_indices_of_ops(vector<GlobalOperator> &ops, const vector<int> &map_precond_var_id_to_new_var_id, const vector<int> &map_eff_var_id_to_new_var_id)
    {
        // Adjust indices in preconditions and effects of all operators in ops vector
        for (size_t op_no = 0; op_no < ops.size(); op_no++) {
            vector<GlobalCondition> &conditions = ops[op_no].get_preconditions();
            for (size_t cond_no = 0; cond_no < conditions.size(); cond_no++) {
                conditions[cond_no].var = map_precond_var_id_to_new_var_id[conditions[cond_no].var];
            }

            vector<GlobalEffect> &effects = ops[op_no].get_effects();
            for (size_t eff_no = 0; eff_no < effects.size(); eff_no++) {
                effects[eff_no].var = map_eff_var_id_to_new_var_id[effects[eff_no].var];
            }

            // Sort the conditions and effects by their respective var id
            sort(conditions.begin(), conditions.end(), cond_comp_func);
            sort(effects.begin(), effects.end(), eff_comp_func);
        }
    }

    void StackelbergTask::check_fix_vars_attacker_preconditioned()
    {
        cout << "Begin check_fix_vars_attacker_preconditioned()..." << endl;

        vector<bool> is_fix_var_attacker_preconditioned(fix_variable_domain.size(),
                                                        false);

        for (size_t op_no = 0; op_no < follower_operators_with_fix_vars_preconds.size();
             op_no++) {
            const vector<GlobalCondition> &conditions =
                follower_operators_with_fix_vars_preconds[op_no].get_preconditions();

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


    StackelbergTask::StackelbergTask() {

        sort_operators();

        if (num_leader_operators == 0) {
            // If there are no fix actions, exit with an error
            cerr << "Error: running stackelberg search on a task without fix actions" << endl;
            exit_with(EXIT_INPUT_ERROR);
        }

        divide_variables();

        clean_follower_actions();

        create_new_variable_indices();

        check_fix_vars_attacker_preconditioned();

        dump_statistics();
    }

    void StackelbergTask::dump_statistics() const {

        
        cout << "fix_variable_domain.size() = " << fix_variable_domain.size() << endl;
        cout << "fix_vars_attacker_preconditioned.size() = " << fix_vars_attacker_preconditioned.size() << endl;
        
        cout << "follower_operators.size() = " << follower_operators.size() << endl;
        cout << "leader_operators.size() = " << leader_operators.size() << endl;
    }

}
