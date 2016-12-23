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
	PerStateInformation<AttackSearchInfo>* curr_per_state_information;
	Heuristic *default_heuristic;
protected:
    virtual void initialize();
    virtual int compute_heuristic(const GlobalState &state);
public:
    void reinitialize(PerStateInformation<AttackSearchInfo>* per_state_information, SearchSpace* search_space, OpenList<std::pair<StateID, int>>* open_list, GlobalState goal_state);
	void set_curr_per_state_information (PerStateInformation<AttackSearchInfo> *per_state_information) {curr_per_state_information = per_state_information; }
	PerStateInformation<AttackSearchInfo>* get_curr_per_state_information () {return curr_per_state_information; }
    AttackSuccessProbReuseHeuristic(const Options &options);
	virtual ~AttackSuccessProbReuseHeuristic();
};



#endif /* SRC_SEARCH_ATTACK_SUCCESS_PROB_REUSE_HEURISTIC_H_ */
