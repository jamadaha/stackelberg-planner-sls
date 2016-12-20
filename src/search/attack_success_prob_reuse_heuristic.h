/*
 * attack_success_prob_reuse_heuristic.h
 *
 *  Created on: 20.12.2016
 *      Author: Patrick
 */

#ifndef SRC_SEARCH_ATTACK_SUCCESS_PROB_REUSE_HEURISTIC_H_
#define SRC_SEARCH_ATTACK_SUCCESS_PROB_REUSE_HEURISTIC_H_

#include "heuristic.h"
#include "search_space.h"
#include "open_lists/open_list.h"
#include <limits>

struct AttackSearchInfo {
        int attack_plan_prob_cost_heuristic_value;
        AttackSearchInfo(int _attack_plan_prob_cost_heuristic_value = std::numeric_limits<int>::max())
        : attack_plan_prob_cost_heuristic_value(_attack_plan_prob_cost_heuristic_value) { }
};


class AttackSuccessProbReuseHeuristic: public Heuristic {
protected:
    virtual void initialize();
    virtual int compute_heuristic(const GlobalState &state);
public:
    void reinitialize(PerStateInformation<AttackSearchInfo> &per_state_information, SearchSpace* search_space, OpenList<StateID>* open_list, const GlobalState* goal_state);
	AttackSuccessProbReuseHeuristic(const Options &options);
	virtual ~AttackSuccessProbReuseHeuristic();
};



#endif /* SRC_SEARCH_ATTACK_SUCCESS_PROB_REUSE_HEURISTIC_H_ */
