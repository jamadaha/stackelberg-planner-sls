/*
 * sort_fixactions_by_attacker_reward.cpp
 *
 *  Created on: 31.03.2017
 *      Author: Patrick
 */

#include "sort_fixactions_by_attacker_reward.h"

using namespace std;

SortFixActionsByAttackerReward::SortFixActionsByAttackerReward(const vector<int> &positive_values, const vector<int> &reward, const vector<GlobalOperator> &fix_operators, const vector<GlobalOperator> &attack_operators_with_fix_vars_preconds, const std::vector<std::vector<std::vector<const GlobalOperator *>>> &deleting_fix_facts_ops) {
	// TODO Auto-generated constructor stub
	reward_for_attack_op.reserve(attack_operators_with_fix_vars_preconds.size());
	disabled_attack_reward_for_fix_op_id.assign(fix_operators.size(), 0);

	for (size_t op_no = 0; op_no < attack_operators_with_fix_vars_preconds.size(); op_no++) {
		const GlobalOperator &op = g_operators[op_no];
		int op_reward = 0;
		for (const auto &e : op.get_effects()) {
			int var = e.var;
			int val = e.val;
			if(val == positive_values[var]) {
				op_reward += reward[var];
			}
		}
		reward_for_attack_op[op.get_op_id()] = op_reward;

		for (const auto &p : op.get_preconditions()) {
			int precond_var = p.var;
			int precond_val = p.val;

            const vector< const GlobalOperator *> &deleting_ops = deleting_fix_facts_ops[precond_var][precond_val];

            for (size_t del_op_no = 0; del_op_no < deleting_ops.size(); del_op_no++) {
            	int deleting_op_id = deleting_ops[del_op_no]->get_op_id();
            	disabled_attack_reward_for_fix_op_id[deleting_op_id] += reward_for_attack_op[op.get_op_id()];
            }
		}
	}
}

SortFixActionsByAttackerReward::~SortFixActionsByAttackerReward() {
	// TODO Auto-generated destructor stub
}

bool SortFixActionsByAttackerReward::operator() (const GlobalOperator *op1, const GlobalOperator *op2) {
	// Sort op with higher disabled attacker reward before op with lower disabled attacker reward
	return disabled_attack_reward_for_fix_op_id[op1->get_op_id()] >= disabled_attack_reward_for_fix_op_id[op2->get_op_id()];
}

void SortFixActionsByAttackerReward::sort_attack_ops(vector<const GlobalOperator *> &attack_ops) {
	sort(attack_ops.begin(), attack_ops.end(), *this);
}

