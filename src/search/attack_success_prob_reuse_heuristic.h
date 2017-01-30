/*
 * attack_success_prob_reuse_heuristic.h
 *
 *  Created on: 20.12.2016
 *      Author: Patrick
 */

#ifndef SRC_SEARCH_ATTACK_SUCCESS_PROB_REUSE_HEURISTIC_H_
#define SRC_SEARCH_ATTACK_SUCCESS_PROB_REUSE_HEURISTIC_H_

#include "heuristic.h"

struct AttackSearchInfo {
        int attack_plan_prob_cost_heuristic_value;
        AttackSearchInfo(int _attack_plan_prob_cost_heuristic_value = std::numeric_limits<int>::max())
        : attack_plan_prob_cost_heuristic_value(_attack_plan_prob_cost_heuristic_value) { }
};

struct AttackSearchSpace {
	PerStateInformation<BudgetSearchNodeInfo> budget_attack_search_node_infos;
	SegmentedVector<AttackSearchInfo> all_attack_search_node_infos;
};


class AttackSuccessProbReuseHeuristic: public Heuristic {
	AttackSearchSpace* curr_attack_search_space;
	Heuristic *default_heuristic;
protected:
    virtual void initialize();
    virtual int compute_heuristic(const GlobalState&) {std::cout << "compute_heuristic() without budget not supported here!" << std::endl; exit(-1);}
    virtual int compute_heuristic(const GlobalState &state, int budget);
	AttackSearchInfo& get_attack_search_info (const GlobalState &state, int budget);
public:
    virtual void reinitialize(AttackSearchSpace* attack_search_space, SearchSpace* search_space, OpenList<std::pair<StateID, int>>* open_list, GlobalState goal_state, int goal_state_budget);
	virtual void set_curr_attack_search_space (AttackSearchSpace* attack_search_space) {curr_attack_search_space = attack_search_space; }
	virtual AttackSearchSpace* get_curr_attack_search_space () {return curr_attack_search_space; }
	virtual void reset();
    AttackSuccessProbReuseHeuristic(const Options &options);
	virtual ~AttackSuccessProbReuseHeuristic();
};



#endif /* SRC_SEARCH_ATTACK_SUCCESS_PROB_REUSE_HEURISTIC_H_ */
