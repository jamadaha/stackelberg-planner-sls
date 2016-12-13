/*
 * global_attack_operator.h
 *
 *  Created on: 13.12.2016
 *      Author: Patrick
 */

#ifndef SRC_SEARCH_GLOBAL_ATTACK_OPERATOR_H_
#define SRC_SEARCH_GLOBAL_ATTACK_OPERATOR_H_

#include "global_operator.h"

class GlobalAttackOperator: public GlobalOperator {
    std::vector<GlobalCondition> fix_preconditions;
    std::vector<GlobalCondition> attack_preconditions;

public:
	GlobalAttackOperator(std::istream &in, bool is_axiom, int cost2);
	virtual ~GlobalAttackOperator();

    const std::vector<GlobalCondition> &get_fix_preconditions() const {return fix_preconditions; }
    const std::vector<GlobalCondition> &get_attack_preconditions() const {return attack_preconditions; }
};

#endif /* SRC_SEARCH_GLOBAL_ATTACK_OPERATOR_H_ */
