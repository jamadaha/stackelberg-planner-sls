#include "partial_order_reduction.h"

#include "stackelberg_task.h"

#include <iostream>
#include <unordered_set>

using namespace std;

namespace stackelberg {

    


    PartialOrderReduction::PartialOrderReduction(const StackelbergTask * task_) : task(task_) {
        compute_commutative_and_dependent_leader_ops_matrices(task);
    }

/**
 * For every pair of ops op1 and op2, this method checks whether op1 has a precond on var v on which op2 has an effect and vice versa.
 * If this is the case, op1 and op2 are not commutative. Only if the precond value is different from effect value, op1 and op2 are dependent.
 * If op1 and op2 both effect a var v with different effect values,
 * there are also not commutative and are dependent. Otherwise, they are commutative and not dependet.
 */
    void PartialOrderReduction::compute_commutative_and_dependent_leader_ops_matrices(const StackelbergTask * task)
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



    void PartialOrderReduction::prune_applicable_leader_ops_sss(const GlobalState &state,
                                                                const vector<int> &follower_plan,
                                                                const vector<const GlobalOperator *> &applicable_ops,
                                                            vector<const GlobalOperator *> &result) {
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
                task->deleting_leader_facts_ops[precond_var][precond_val];

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
                                task->achieving_leader_facts_ops[precond_var][precond_val];

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


    void PartialOrderReduction::get_all_dependent_ops(const GlobalOperator *op,
                                                      vector<const GlobalOperator *> &result) const{
	const auto & leader_operators = task->get_leader_operators();
	for (size_t other_op_no = 0; other_op_no < dependent_leader_ops.size();
	     other_op_no++) {
            if (dependent_leader_ops[op->get_op_id()][other_op_no]) {
		result.push_back(&leader_operators[other_op_no]);
	    }
        }
    }

}
