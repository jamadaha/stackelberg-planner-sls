/*
 * attack_success_prob_reuse_heuristic.h
 *
 *  Created on: 20.12.2016
 *      Author: Patrick
 */

#ifndef SRC_SEARCH_ATTACK_SUCCESS_PROB_REUSE_HEURISTIC_H_
#define SRC_SEARCH_ATTACK_SUCCESS_PROB_REUSE_HEURISTIC_H_

#include "heuristic.h"


class AttackSuccessProbReuseHeuristic: public Heuristic {
protected:
    virtual void initialize();
    virtual int compute_heuristic(const GlobalState &state);
public:
	AttackSuccessProbReuseHeuristic(const Options &options);
	virtual ~AttackSuccessProbReuseHeuristic();
};



#endif /* SRC_SEARCH_ATTACK_SUCCESS_PROB_REUSE_HEURISTIC_H_ */
