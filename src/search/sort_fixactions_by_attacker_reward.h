/*
 * sort_fixactions_by_attacker_reward.h
 *
 *  Created on: 31.03.2017
 *      Author: Patrick
 */

#ifndef SRC_SEARCH_SORT_FIXACTIONS_BY_ATTACKER_REWARD_H_
#define SRC_SEARCH_SORT_FIXACTIONS_BY_ATTACKER_REWARD_H_

#include <vector>
#include "global_operator.h"

class SortFixActionsByAttackerReward {
	std::vector<int> reward_for_attack_op;
	std::vector<int> disabled_attack_reward_for_fix_op_id;
public:
	SortFixActionsByAttackerReward(const std::vector<int> &positive_values, const std::vector<int> &reward, const std::vector<GlobalOperator> &fix_operators, const std::vector<GlobalOperator> &attack_operators_with_fix_vars_preconds, const std::vector<std::vector<std::vector<const GlobalOperator *>>> &deleting_fix_facts_ops);
	virtual ~SortFixActionsByAttackerReward();
	bool op_ptr_reward_comp(const GlobalOperator *op1, const GlobalOperator *op2);
};


#endif /* SRC_SEARCH_SORT_FIXACTIONS_BY_ATTACKER_REWARD_H_ */
