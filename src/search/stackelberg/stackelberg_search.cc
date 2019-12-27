#include "stackelberg_search.h"
#include <vector>
#include "../option_parser.h"
#include "../plugin.h"
#include "../attack_success_prob_reuse_heuristic.h"
#include "../eager_search.h"
#include "../budget_dead_end_heuristic.h"

#include <cassert>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include "../utils/timer.h"

using namespace std;

namespace stackelberg {


    StackelbergSearch::StackelbergSearch(const Options &opts) :
        SearchEngine(opts),
        follower_budget_factor (opts.get<double>("follower_budget_factor")),
        leader_budget_factor (opts.get<double>("leader_budget_factor")),
        use_partial_order_reduction (opts.get<bool>("partial_order_reduction")),
        check_parent_follower_plan_applicable (opts.get<bool>("check_parent_follower_plan_applicable")),
        check_leader_state_already_known (opts.get<bool>("check_leader_state_already_known")),
        do_follower_op_dom_pruning (opts.get<bool>("follower_op_dom_pruning")),
        use_IDS (opts.get<bool>("ids")),
        sort_leader_ops_advanced (opts.get<bool>("sort_leader_ops")),
        upper_bound_pruning (opts.get<bool>("upper_bound_pruning")) {
        search_engine = opts.get<SearchEngine *>("search_engine");


        if (opts.contains("follower_heuristic")) {
            follower_heuristic =
                opts.get<Heuristic *>("follower_heuristic"); // (AttackSuccessProbReuseHeuristic*) (((BudgetDeadEndHeuristic*)opts.get<Heuristic*>("follower_heuristic"))->get_prob_cost_heuristic());
        } else {
            follower_heuristic = nullptr;
        }

        g_initial_budget = opts.get<int>("initial_follower_budget");
        if (g_initial_budget < UNLTD_BUDGET) {
            g_initial_budget = (int) ((double) g_initial_budget) * follower_budget_factor;
        }

        max_leader_actions_budget  = opts.get<int>("initial_leader_budget");
        if(max_leader_actions_budget < UNLTD_BUDGET) {
            max_leader_actions_budget = (int) ((double) max_leader_actions_budget) * leader_budget_factor;
        }

    }

    void StackelbergSearch::initialize()
    {
        cout << "Initializing StackelbergSearch..." << endl;

        auto t1 = chrono::high_resolution_clock::now();

        task = make_unique<StackelbergTask> ();

	
        // Creating two new state_registries, one locally only for fix variables and one globally only for attack variables

	
        if(check_parent_follower_plan_applicable) {
            leader_search_node_infos_follower_plan.set_relevant_variables(task->get_leader_vars_follower_preconditioned());
        } else {

	    size_t num_leader_vars = task->get_leader_variable_domain().size();
            vector<int> temp;
            for (size_t var = 0; var < num_leader_vars; var++) {
                temp.push_back(var);
            }
            leader_search_node_infos_follower_plan.set_relevant_variables(temp);
        }


        leader_operators_successor_generator.reset(
	    create_successor_generator(
		task->get_leader_variable_domain(), task->get_leader_operators(), task->get_leader_operators()));
	
        /* FIXME REMOVED DIVIDING VARIABLES
           follower_operators_for_leader_vars_successor_generator = create_successor_generator(
           leader_variable_domain,
           follower_operators_with_leader_vars_preconds, follower_operators);
        */

        compute_commutative_and_dependent_leader_ops_matrices();

        if (use_partial_order_reduction || upper_bound_pruning) {
            compute_leader_facts_ops_sets();
        }

        // if (do_follower_op_dom_pruning) {
        //     compute_op_dominance_relation(follower_operators, dominated_follower_op_ids);
        // }

        
        /* FIXME REMOVED DIVIDING VARIABLES
           g_operators.clear();
           g_operators.insert(g_operators.begin(),
           follower_operators.begin(), follower_operators.end());
           g_follower_op_included.resize(follower_operators.size(), true);
        */


        if (upper_bound_pruning) {
            g_operators.clear();
            compute_always_applicable_follower_ops(g_operators);
            cout << "number of always applicable attack ops: " << g_operators.size() << endl;
    	    delete g_successor_generator;
    	    g_successor_generator = create_successor_generator(g_variable_domain, g_operators, g_operators);
    	    search_engine->reset();
    	    g_state_registry->reset();
    	    if(follower_heuristic) {
                follower_heuristic->reset();
    	    }
            
    	    search_engine->search();
    	    if (search_engine->found_solution()) {
                search_engine->save_plan_if_necessary();
                follower_cost_upper_bound = search_engine->calculate_plan_cost();
    	    } else {
                follower_cost_upper_bound = StackelbergTask::FOLLOWER_TASK_UNSOLVABLE;
    	    }
    	    cout << "follower_cost_upper_bound: " << follower_cost_upper_bound << endl;
        }

        /* FIXME Because of REMOVED DIVIDING VARIABLES, we added this: */
        g_operators.clear();
        for (const auto & op :task->get_follower_operators_with_all_preconds()) {
            g_operators.push_back(op);
        }
        delete g_successor_generator;
        g_successor_generator = create_successor_generator(g_variable_domain, g_operators, g_operators);
        if(follower_heuristic) {
            follower_heuristic->reset();
        }

        chrono::high_resolution_clock::time_point t2 =
            chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();
        leader_search_initialize_duration = duration;
    }



/**
 * returns a SuccessorGeneratorSwitch based on the preconditions of the ops in pre_cond_ops and entailing the ops from ops vector in the leaves
 */
    SuccessorGeneratorSwitch * StackelbergSearch::create_successor_generator(
        const vector<int> &variable_domain,
        const vector<GlobalOperator> &pre_cond_ops, const vector<GlobalOperator> &ops)
    {
#ifdef LEADER_SEARCH_DEBUG
        cout << "Begin create_successor_generator..." << endl;
#endif

        int root_var_index = 0;

        auto root_node = new SuccessorGeneratorSwitch (root_var_index, variable_domain[root_var_index]);

        for (size_t op_no = 0; op_no < pre_cond_ops.size(); op_no++) {
            /*#ifdef LEADER_SEARCH_DEBUG
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
    void StackelbergSearch::compute_commutative_and_dependent_leader_ops_matrices()
    {
	cout << "Begin compute_commutative_and_dependent_leader_ops_matrices()..." << endl;

	/*   cout << "We assume that all fix actions are commutative and not dependent!" <<
	     endl;
	     {
	     vector<bool> val1(leader_operators.size(), true);
	     commutative_leader_ops.assign(leader_operators.size(), val1);
	     vector<bool> val2(leader_operators.size(), false);
	     dependent_leader_ops.assign(leader_operators.size(), val2);
	     return;
	     }*/


	const auto & leader_operators = task->get_leader_operators();
	vector<bool> val(leader_operators.size());
	commutative_leader_ops.assign(leader_operators.size(), val);
	dependent_leader_ops.assign(leader_operators.size(), val);
	for (size_t op_no1 = 0; op_no1 < leader_operators.size(); op_no1++) {
            for (size_t op_no2 = op_no1 + 1; op_no2 < leader_operators.size(); op_no2++) {
		/*#ifdef LEADER_SEARCH_DEBUG
		  cout << "Comparing op1 with id " << op_no1 << ":" << endl;
		  leader_operators[op_no1].dump();
		  cout << "to op2 with id " << op_no2 << ":" << endl;
		  leader_operators[op_no2].dump();
		  #endif*/

		const vector<GlobalCondition> &conditions1 =
		    leader_operators[op_no1].get_preconditions();
		const vector<GlobalCondition> &conditions2 =
		    leader_operators[op_no2].get_preconditions();
		const vector<GlobalEffect> &effects1 = leader_operators[op_no1].get_effects();
		const vector<GlobalEffect> &effects2 = leader_operators[op_no2].get_effects();

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

		/*#ifdef LEADER_SEARCH_DEBUG
		  cout << "ops are commutative?: " << commutative << endl;
		  cout << "ops are dependent?: " << dependent << endl;
		  #endif*/
		commutative_leader_ops[op_no1][op_no2] = commutative;
		commutative_leader_ops[op_no2][op_no1] = commutative;

		dependent_leader_ops[op_no1][op_no2] = dependent;
		dependent_leader_ops[op_no2][op_no1] = dependent;
	    }
        }
    }

    void StackelbergSearch::compute_op_dominance_relation(const
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

		/*#ifdef LEADER_SEARCH_DEBUG
		  cout << "op1 dominates op2?: " << dominated_or_equivalent << endl;
		  //#endif*/
		if (dominated_or_equivalent) {
		    dominated_op_ids[op_no1].push_back(op_no2);
		}
	    }
        }
    }

    void StackelbergSearch::compute_leader_facts_ops_sets()
    {
	cout << "Begin compute_leader_facts_ops_sets()..." << endl;

	int num_leader_vars = task->get_num_leader_vars();
	const auto & leader_variable_domain = task->get_leader_variable_domain();
	const auto & leader_operators = task->get_leader_operators();
	deleting_leader_facts_ops.resize(num_leader_vars);
	achieving_leader_facts_ops.resize(num_leader_vars);

	for (int var = 0; var < num_leader_vars; var++) {
            int domain_size = leader_variable_domain[var];
            deleting_leader_facts_ops[var].resize(domain_size);
            achieving_leader_facts_ops[var].resize(domain_size);
        }

	for (size_t op_no = 0; op_no < leader_operators.size(); op_no++) {
            const GlobalOperator *op = &leader_operators[op_no];

            const vector<GlobalEffect> &effects = op->get_effects();
            for (size_t eff_no = 0; eff_no < effects.size(); eff_no++) {
		int var = effects[eff_no].var;
		int effect_val = effects[eff_no].val;

		achieving_leader_facts_ops[var][effect_val].push_back(op);

		for (int other_val = 0; other_val < leader_variable_domain[var]; other_val++) {
		    if (other_val != effect_val) {
			deleting_leader_facts_ops[var][other_val].push_back(op);
		    }
		}
	    }
        }
    }

    void StackelbergSearch::get_all_dependent_ops(const GlobalOperator *op,
						  vector<const GlobalOperator *> &result)
    {
	const auto & leader_operators = task->get_leader_operators();
	for (size_t other_op_no = 0; other_op_no < dependent_leader_ops.size();
	     other_op_no++) {
            if (dependent_leader_ops[op->get_op_id()][other_op_no]) {
		result.push_back(&leader_operators[other_op_no]);
	    }
        }
    }

            void StackelbergSearch::prune_applicable_leader_ops_sss(const GlobalState &state,
                const vector<int> &follower_plan,
                const vector<const GlobalOperator *> &applicable_ops,
                vector<const GlobalOperator *> &result)
            {
            unordered_set<const GlobalOperator *> applicable_ops_set(applicable_ops.begin(),
                applicable_ops.end());

	    const auto & follower_operators_with_leader_vars_preconds = task->get_follower_operators_with_leader_vars_preconds();
	    
            vector<const GlobalOperator *> current_T_s;
            vector<bool> is_in_current_T_s(task->get_leader_operators().size(), false);
            // Initialize T_s to the disjunctive action landmark
            // We rely on g_plan containing the currently computed follower plan
            for (size_t op_no = 0; op_no < follower_plan.size(); op_no++) {
            int op_id = follower_plan[op_no];
            const GlobalOperator *op = & follower_operators_with_leader_vars_preconds[op_id];

            const vector<GlobalCondition> &preconditions = op->get_preconditions();
            for (size_t precond_no = 0; precond_no < preconditions.size(); precond_no++) {
            int precond_var = preconditions[precond_no].var;
            int precond_val = preconditions[precond_no].val;
            const vector< const GlobalOperator *> &deleting_ops =
                deleting_leader_facts_ops[precond_var][precond_val];

            for (size_t del_op_no = 0; del_op_no < deleting_ops.size(); del_op_no++) {
                if (!is_in_current_T_s[deleting_ops[del_op_no]->get_op_id()]) {
                    current_T_s.push_back(deleting_ops[del_op_no]);
                    is_in_current_T_s[deleting_ops[del_op_no]->get_op_id()] = true;
                }
            }
            }
            }


	    const auto & leader_operators = task->get_leader_operators();
	    
            vector<bool> is_in_result(leader_operators.size(), false);

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
                                achieving_leader_facts_ops[precond_var][precond_val];

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

void StackelbergSearch::prune_dominated_ops(vector<const GlobalOperator *> &ops,
                                            vector<vector<int>> dominated_op_ids)
{
    vector<bool> marked_for_erase(dominated_op_ids.size(), false);
    vector<int> dominated_by(dominated_op_ids.size(), -1);

    //cout << "follower_ops:" << endl;

    for (size_t op_no = 0; op_no < ops.size(); op_no++) {
        //follower_ops[op_no]->dump();
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
              follower_operators[dominated_by[(*it)->get_op_id()]].dump();*/

            it = ops.erase(it);
        } else {
            ++it;
        }
    }
}

void StackelbergSearch::compute_always_applicable_follower_ops(vector<GlobalOperator> &ops) {

    
    for (const auto & op : task->get_follower_operators_with_leader_vars_preconds()) {

        bool always_applicable = true;
        const vector<GlobalCondition> &preconditions = op.get_preconditions();
        for (size_t precond_no = 0; precond_no < preconditions.size(); precond_no++) {
            int precond_var = preconditions[precond_no].var;
            int precond_val = preconditions[precond_no].val;
            const vector< const GlobalOperator *> &deleting_ops =
                deleting_leader_facts_ops[precond_var][precond_val];

            if(deleting_ops.size() > 0) {
                always_applicable = false;
                break;
            }
        }

        if (always_applicable) {
            ops.push_back(task->get_follower_operators_with_all_preconds()[op.get_op_id()]);
        }
    }
}

bool op_ptr_name_comp(const GlobalOperator *op1, const GlobalOperator *op2)
{
    return op1->get_name() < op2->get_name();
}

string StackelbergSearch::ops_to_string(vector<const GlobalOperator *> &ops) {
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


int StackelbergSearch::compute_pareto_frontier(const GlobalState &state,
                                               vector<const GlobalOperator *> &leader_ops_sequence, int leader_actions_cost,
                                               const vector<int> &parent_follower_plan, int parent_follower_plan_cost,
                                               vector<int> &sleep, bool recurse)
{

    const auto & follower_operators_with_leader_vars_preconds = task->get_follower_operators_with_leader_vars_preconds();
    num_recursive_calls++;
#ifdef LEADER_SEARCH_DEBUG
    if ((num_recursive_calls % 50) == 0) {
        cout << num_recursive_calls << " num recursive calls until now." << endl;
        pareto_frontier.dump();
    }
#endif

#ifdef LEADER_SEARCH_DEBUG
    cout << "in call of compute_pareto_frontier for state: " << endl;
    //state.dump_fdr(leader_variable_domain, leader_variable_name);
    cout << "with id: " << state.get_id().hash() << endl;
    cout << "and current fix actions op_sequence: " << endl;
    for (size_t i = 0; i < leader_ops_sequence.size(); i++) {
        leader_ops_sequence[i]->dump();
    }

    cout << "leader_actions_cost: " << leader_actions_cost << endl;
#endif
    if (!recurse) {
#ifdef LEADER_SEARCH_DEBUG
        cout << "We won't recurse in this execution of compute_pareto_frontier" << endl;
#endif
        num_recursive_calls_for_sorting++;
    }

    bool parent_follower_plan_applicable = false;
    if (check_parent_follower_plan_applicable && parent_follower_plan.size() > 0) {
        /*cout << "parent attack plan: " << endl;
          for (size_t op_no = 0; op_no < parent_follower_plan.size(); op_no++) {
          follower_operators_with_leader_vars_preconds[parent_follower_plan[op_no]].dump();
          }*/

        parent_follower_plan_applicable = true;
        // Check whether parent_follower_plan is still applicable in current fix-state
        for (size_t op_no = 0; op_no < parent_follower_plan.size(); op_no++) {
            if (!follower_operators_with_leader_vars_preconds[parent_follower_plan[op_no]].is_applicable(
                    state)) {
                parent_follower_plan_applicable = false;
                break;
            }
        }
    }

    vector<int> follower_plan;

    AttackSearchSpace *follower_heuristic_search_space = NULL;
    bool free_follower_heuristic_per_state_info = false;

    int follower_plan_cost = StackelbergTask::FOLLOWER_TASK_UNSOLVABLE;
    FixSearchInfoAttackPlan &info_follower_plan =
        leader_search_node_infos_follower_plan[state];
    FixSearchInfoFixSequence &info_leader_sequence =
        leader_search_node_infos_leader_sequence[state];

    if ((check_leader_state_already_known
         && info_follower_plan.follower_plan_prob_cost != -1)
        || parent_follower_plan_applicable) {
        if (check_leader_state_already_known
            && info_follower_plan.follower_plan_prob_cost != -1) {
            follower_plan_cost = info_follower_plan.follower_plan_prob_cost;
            follower_plan = info_follower_plan.follower_plan;
            spared_follower_searches_because_leader_state_already_seen++;

#ifdef LEADER_SEARCH_DEBUG
            cout << "Attack prob cost for this state is already known in PerStateInformation: "
                 << follower_plan_cost << endl;
#endif
            if (info_leader_sequence.leader_actions_cost != -1
                && leader_actions_cost > info_leader_sequence.leader_actions_cost
                && info_leader_sequence.already_in_frontier) {
#ifdef LEADER_SEARCH_DEBUG
                cout << "Current fix action sequence is more expensive than already known sequence... don't make further recursive calls. "
                     << endl;
#endif
                num_leader_op_paths++;
                return follower_plan_cost;
            } else {
            	info_leader_sequence.leader_actions_cost = leader_actions_cost;
            }
        } else {
            follower_plan_cost = parent_follower_plan_cost;
            if (check_leader_state_already_known) {
                info_follower_plan.follower_plan_prob_cost = follower_plan_cost;
                info_follower_plan.follower_plan = parent_follower_plan;
                info_leader_sequence.leader_actions_cost = leader_actions_cost;
            }
            spared_follower_searches_because_parent_plan_applicable++;
#ifdef LEADER_SEARCH_DEBUG
            cout << "Attack prob cost for this state is already known from parent_follower_plan: "
                 << follower_plan_cost << endl;
#endif
        }

        if (follower_heuristic) {
            follower_heuristic_search_space = follower_heuristic->get_curr_attack_search_space();
        }
    } else {
        num_follower_searches++;

        /* FIXME REMOVED DIVIDING VARIABLES
           vector<const GlobalOperator *> applicable_follower_operators;
           follower_operators_for_leader_vars_successor_generator->generate_applicable_ops(
           state, applicable_follower_operators);


           if (do_follower_op_dom_pruning) {
           prune_dominated_ops(applicable_follower_operators, dominated_follower_op_ids);
           }

           g_operators.clear();
           g_follower_op_included.assign(follower_operators.size(), false);
           //cout << "g_operators: " << endl;
           for (size_t op_no = 0; op_no < applicable_follower_operators.size(); op_no++) {
           g_operators.push_back(*applicable_follower_operators[op_no]);
           g_follower_op_included[applicable_follower_operators[op_no]->get_op_id()] = true;
           //all_follower_operators[op_no]->dump();
           }

           delete g_successor_generator;
           g_successor_generator = create_successor_generator(g_variable_domain, g_operators, g_operators);
           //g_successor_generator->dump();
           //cout << "Follower dump everything: " << endl;
           //dump_everything();

           */

        /* FIXME Because REMOVED DIVIDING VARIABLES, adjust follower initial state w.r.t fix variables: */
	const auto & leader_variable_domain = task->get_leader_variable_domain();
        for (size_t leader_var = 0; leader_var < leader_variable_domain.size(); leader_var++) {
            int orig_var_id = task->get_map_leader_var_id_to_orig_var_id()[leader_var];
            g_initial_state_data[orig_var_id] = state[leader_var];
        }

        chrono::high_resolution_clock::time_point tt1 =
            chrono::high_resolution_clock::now();
        search_engine->reset();
        g_state_registry->reset();
        if (follower_heuristic != NULL) {
            //follower_heuristic->reset();
        }
        chrono::high_resolution_clock::time_point tt2 =
            chrono::high_resolution_clock::now();
        auto duration2 = chrono::duration_cast<chrono::milliseconds>(tt2 - tt1).count();
        reset_and_initialize_duration_sum += duration2;

        // Call search, but make sure that no stuff is written to cout when we are not debugging
#ifndef LEADER_SEARCH_DEBUG
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
        follower_search_duration_sum += duration;

#ifndef LEADER_SEARCH_DEBUG
        cout.rdbuf(old);   			// <-- restore
#endif

        SearchSpace *search_space = search_engine->get_search_space();
        all_follower_states += search_space->get_num_search_node_infos();

        if (search_engine->found_solution()) {
            search_engine->save_plan_if_necessary();
            follower_plan_cost = search_engine->calculate_plan_cost();
#ifdef LEADER_SEARCH_DEBUG
            cout << "Attack follower_plan cost is " << follower_plan_cost << endl;
#endif

            if (follower_heuristic) {
                follower_heuristic_search_space = new AttackSearchSpace();
                free_follower_heuristic_per_state_info = true;
                
                OpenList<pair<StateID, int>> *open_list = ((EagerSearch *)
                                                           search_engine)->get_open_list();
                const GlobalState *goal_state = search_engine->get_goal_state();
                const int goal_state_budget = search_engine->get_goal_state_budget();

                follower_heuristic_search_space->budget_attack_search_node_infos.set_relevant_variables(task->get_follower_vars_indizes());

                follower_heuristic->reinitialize(follower_heuristic_search_space, search_space,
						 open_list, *goal_state,
						 goal_state_budget);
            }
        } else {
#ifdef LEADER_SEARCH_DEBUG
            cout << "Follower task was not solvable!" << endl;
#endif
            follower_plan_cost = StackelbergTask::FOLLOWER_TASK_UNSOLVABLE;
        }
        if (check_leader_state_already_known) {
            info_follower_plan.follower_plan_prob_cost = follower_plan_cost;
            info_leader_sequence.leader_actions_cost = leader_actions_cost;
        }
    }

    // Copy found g_plan to local vector iff we even performed a successful search
    if (!parent_follower_plan_applicable && follower_plan.empty()
        && follower_plan_cost != StackelbergTask::FOLLOWER_TASK_UNSOLVABLE) {
        for (size_t op_no = 0; op_no < g_plan.size(); op_no++) {
            follower_plan.push_back(g_plan[op_no]->get_op_id());
        }
        if (check_leader_state_already_known) {
            info_follower_plan.follower_plan = follower_plan;

            /*vector<const GlobalOperator *> ops(follower_plan.size());
              for (size_t op_no = 0; op_no < follower_plan.size(); op_no++) {
              ops[op_no] = &follower_operators[follower_plan[op_no]];
              }
              cerr << leader_state_to_string(state) << ": " << ops_to_string(ops) << endl;*/

        }
    }

    if (!recurse) {
        return follower_plan_cost;
    }

    if (!info_leader_sequence.already_in_frontier) {
        pareto_frontier.add_node(leader_actions_cost, follower_plan_cost, leader_ops_sequence, parent_follower_plan_applicable ? parent_follower_plan : follower_plan);
        info_leader_sequence.already_in_frontier = true;
    }

    if (follower_plan_cost == StackelbergTask::FOLLOWER_TASK_UNSOLVABLE) {
        // Return, if follower task was not solvable
        num_leader_op_paths++;
        return follower_plan_cost;
    }


    vector<const GlobalOperator *> applicable_ops;
    leader_operators_successor_generator->generate_applicable_ops(state,
                                                               applicable_ops);


    //sort(applicable_ops.begin(), applicable_ops.end(), op_ptr_name_comp);

    /*cout << "applicable ops: " << endl;
      for (size_t op_no = 0; op_no < applicable_ops.size(); op_no++) {
      applicable_ops[op_no]->dump();
      }*/

    vector<const GlobalOperator *> applicable_ops_after_pruning;
    if (use_partial_order_reduction) {
        prune_applicable_leader_ops_sss(state,
                                     parent_follower_plan_applicable ? parent_follower_plan : follower_plan,
                                     applicable_ops, applicable_ops_after_pruning);
    } else {
        applicable_ops_after_pruning.swap(applicable_ops);
    }

    /*cout << "applicable ops after pruning: " << endl;
      for (size_t op_no = 0; op_no < applicable_ops_after_pruning.size(); op_no++) {
      applicable_ops_after_pruning[op_no]->dump();
      }*/

    // if (sortFixActionsByFollowerReward != NULL) {
    //     cout << "Sort fix ops!" << endl;
    //     sortFixActionsByFollowerReward->sort_leader_ops(applicable_ops);
    //     cout << "After sorting fix ops" << endl;
    // }


    if (sort_leader_ops_advanced) {
	const auto & leader_operators = task->get_leader_operators();
        vector<int> recursive_follower_costs(leader_operators.size(), 0);

        iterate_applicable_ops(applicable_ops_after_pruning, state,
                               parent_follower_plan_applicable ? parent_follower_plan : follower_plan,
                               follower_plan_cost, leader_ops_sequence, sleep, follower_heuristic_search_space, false,
                               recursive_follower_costs);

        struct myComp {
            const vector<int> &recursive_follower_costs;
            myComp(const vector<int> &_recursive_follower_costs):
                recursive_follower_costs(_recursive_follower_costs) {}
            bool operator()(const GlobalOperator *op1, const GlobalOperator *op2)
                {
                    if (recursive_follower_costs[op1->get_op_id()] ==
                        recursive_follower_costs[op2->get_op_id()]) {
                        return op1->get_cost() < op2->get_cost();
                    } else {
                        return recursive_follower_costs[op1->get_op_id()] >
                            recursive_follower_costs[op2->get_op_id()];
                    }
                }
        } myobject(recursive_follower_costs);

        sort(applicable_ops_after_pruning.begin(), applicable_ops_after_pruning.end(),
             myobject);
    }

    vector<int> dummy;

    // cerr << leader_state_to_string(state) << ": " << ops_to_string(applicable_ops_after_pruning) << endl;

    iterate_applicable_ops(applicable_ops_after_pruning, state,
                           parent_follower_plan_applicable ? parent_follower_plan : follower_plan,
                           follower_plan_cost, leader_ops_sequence, sleep, follower_heuristic_search_space, true,
                           dummy);

    if (free_follower_heuristic_per_state_info) {
        delete follower_heuristic_search_space;
    }

    return follower_plan_cost;
}

void StackelbergSearch::iterate_applicable_ops(const
                                               vector<const GlobalOperator *> &applicable_ops,
                                               const GlobalState &state, const vector<int> &follower_plan, int follower_plan_cost,
                                               vector<const GlobalOperator *> &leader_ops_sequence,
                                               vector<int> &sleep, AttackSearchSpace *follower_heuristic_search_space,
                                               bool recurse, vector<int> &recursive_follower_costs)
{
    /*cout << "applicable ops after pruning: " << endl;
      for (size_t op_no = 0; op_no < applicable_ops.size(); op_no++) {
      applicable_ops[op_no]->dump();
      }*/
    bool at_least_one_recursion = false;

    //vector<const GlobalOperator *> actually_recursed_ops;

    for (size_t op_no = 0; op_no < applicable_ops.size(); op_no++) {
        const GlobalOperator *op = applicable_ops[op_no];
        if (find(leader_ops_sequence.begin(), leader_ops_sequence.end(),
                 applicable_ops[op_no])
            != leader_ops_sequence.end()) {
            // Continue, if op is already in sequence
            continue;
        }
        if (sleep[op->get_op_id()] != 0) {
            // Continue, if op is in sleep set
            continue;
        }
        leader_ops_sequence.push_back(op);
        int new_leader_actions_cost = task->calculate_leader_actions_plan_cost(leader_ops_sequence);
        if (new_leader_actions_cost > curr_leader_actions_budget) {
            // Continue, if adding this op exceeds the budget
            leader_ops_sequence.pop_back();
            returned_somewhere_bc_of_budget = true;
            continue;
        }
        if (upper_bound_pruning && new_leader_actions_cost > leader_action_costs_for_follower_upper_bound) {
            //cout
            //       << "Do not continue with this op, because the new leader_action_cost is already greater than leader_action_costs_for_follower_upper_bound"
            //        << endl;
            leader_ops_sequence.pop_back();
            continue;
        }

        //actually_recursed_ops.push_back(op);

        // Add all ops before op_no in applicable_ops to sleep set if they are commutative
        int op_id = op->get_op_id();
        for (size_t op_no2 = 0; op_no2 < op_no; op_no2++) {
            int op2_id = applicable_ops[op_no2]->get_op_id();
            if (commutative_leader_ops[op_id][op2_id]) {
                sleep[op2_id]++;
            }
        }
        const GlobalState &next_state = leader_vars_state_registry->get_successor_state(
            state, *op);
        if (follower_heuristic != NULL) {
            follower_heuristic->set_curr_attack_search_space(follower_heuristic_search_space);
        }

        at_least_one_recursion = true;

 
        int follower_cost = compute_pareto_frontier(next_state, leader_ops_sequence,
                                                    new_leader_actions_cost,
                                                    follower_plan, follower_plan_cost, sleep, recurse);
        if (!recurse) {
            recursive_follower_costs[op->get_op_id()] = follower_cost;
        }
        // Remove all ops before op_no in applicable_ops from sleep set if they are commutative
        for (size_t op_no2 = 0; op_no2 < op_no; op_no2++) {
            int op2_id = applicable_ops[op_no2]->get_op_id();
            if (commutative_leader_ops[op_id][op2_id]) {
                sleep[op2_id]--;
            }
        }
        leader_ops_sequence.pop_back();
    }
    if (!at_least_one_recursion) {
        num_leader_op_paths++;
    }
    /* if(recurse && at_least_one_recursion) {
       cerr << leader_state_to_string(state) << ": " << ops_to_string(actually_recursed_ops) << endl;
       }*/
}



SearchStatus StackelbergSearch::step() {
    cout << "Starting fix-actions IDS..." << endl;

    if(use_IDS) {
        curr_leader_actions_budget = max(2, max_leader_action_cost);
    } else {
        curr_leader_actions_budget = max_leader_actions_budget;
    }

    const auto & leader_operators = task->get_leader_operators();
    auto t1 = chrono::high_resolution_clock::now();

    while (true) {
        cout << "(Re)starting search with fix action budget: " << curr_leader_actions_budget << endl;

        vector<const GlobalOperator *> op_sequnce;
        vector<int> parent_follower_plan;
        vector<int> sleep(leader_operators.size(), 0);
        returned_somewhere_bc_of_budget = false;

        compute_pareto_frontier(leader_vars_state_registry->get_initial_state(), op_sequnce, 0, parent_follower_plan, 0, sleep, true);

        if(pareto_frontier.is_complete(StackelbergTask::FOLLOWER_TASK_UNSOLVABLE)) {
            break;
        }

        if(!returned_somewhere_bc_of_budget) {
            break;
        }

        if(curr_leader_actions_budget >= max_leader_actions_budget) {
            break;
        }

        int new_leader_actions_budget = curr_leader_actions_budget * ids_leader_budget_factor;
        if (new_leader_actions_budget < 0) {
            new_leader_actions_budget = max_leader_actions_budget;
        }

        curr_leader_actions_budget = min(new_leader_actions_budget , max_leader_actions_budget);
    }

    auto t2 = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>( t2 - t1 ).count();

    cout << "total time: " << g_timer << endl;
    cout << "FixSearch initialize took: " << leader_search_initialize_duration << "ms"
         << endl;
    cout << "Complete Fixsearch took: " << duration << "ms" << endl;
    cout << "Search in Follower Statespace took " << (follower_search_duration_sum/1000) <<
        "ms" << endl;
    cout << "Search in Fixactions Statespace took " << (duration -
                                                        (follower_search_duration_sum/1000)) << "ms" << endl;
    cout << "reset_and_initialize_duration_sum: " <<
        reset_and_initialize_duration_sum << "ms" << endl;
    cout << "They were in total " << num_recursive_calls <<
        " calls to compute_pareto_frontier." << endl;
    cout << "thereof because of sorting fix actions: " <<
        num_recursive_calls_for_sorting << endl;
    cout << "and " << (num_recursive_calls - num_recursive_calls_for_sorting) <<
        " \"real\" calls" << endl;
    cout << "They were " << num_follower_searches <<
        " searches in Follower Statespace" << endl;
    cout << "We spared " << (spared_follower_searches_because_leader_state_already_seen
                             - (num_recursive_calls - num_recursive_calls_for_sorting) + 1)
         << " follower searches, because the fix state was already known" << endl;
    cout << "We spared " << spared_follower_searches_because_parent_plan_applicable
         << " follower searches, because the fix parent state attack plan was still applicable"
         << endl;
    // cout << "Follower Searchspace had " << (all_follower_states / num_follower_searches) << " states on average" << endl;
    // cout << "Follower Searchspaces accumulated " << g_state_registry->size() << " states in state_registry" << endl;
    // cout << "Num fix action paths: " << num_leader_op_paths << endl; TODO This is currently not computed correctly
    pareto_frontier.dump(*task);
    exit(0);
    return IN_PROGRESS;
}


SearchEngine *_parse(OptionParser &parser) {
    SearchEngine::add_options_to_parser(parser);
    
    parser.add_option<SearchEngine *>("search_engine");
    parser.add_option<Heuristic *>("follower_heuristic",
                                   "The heuristic used for search in FollowerStateSpace", "", OptionFlags(false));
    parser.add_option<int>("initial_follower_budget", "The initial follower Budget",
                           "2147483647");
    parser.add_option<int>("initial_leader_budget", "The initial fix actions Budget",
                           "2147483647");
    parser.add_option<double>("follower_budget_factor",
                              "The factor to multiply with attack actions budget", "1.0");
    parser.add_option<double>("leader_budget_factor",
                              "The factor to multiply with fix actions budget", "1.0");
    parser.add_option<bool>("partial_order_reduction",
                            "use partial order reduction for fix ops", "true");
    parser.add_option<bool>("check_parent_follower_plan_applicable",
                            "always check whether the follower plan of the parent fix state is still applicable",
                            "true");
    parser.add_option<bool>("check_leader_state_already_known",
                            "always check whether the current fix state is already known and spare search in follower statespace",
                            "true");
    parser.add_option<bool>("follower_op_dom_pruning",
                            "use the attack operator dominance pruning", "false");
    parser.add_option<bool>("sort_leader_ops",
                            "When expanding fix state successors, first compute follower cost in all successor states and then recurse in descending cost order.",
                            "true");
    parser.add_option<bool>("ids", "use iterative deepening search", "true");
    parser.add_option<bool>("upper_bound_pruning", "Prune fix action sequences with higher costs then already known sequences leading to a state with upper bound follower costs", "true");
    Options opts = parser.parse();
    if (!parser.dry_run()) {
        return new StackelbergSearch(opts);
    }
    return NULL;
}

Plugin<SearchEngine> _plugin("stackelberg", _parse);
}
