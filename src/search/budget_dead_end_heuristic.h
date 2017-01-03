/*
 * budget_dead_end_heuristic.h
 *
 *  Created on: 03.01.2017
 *      Author: Patrick
 */

#ifndef SRC_SEARCH_BUDGET_DEAD_END_HEURISTIC_H_
#define SRC_SEARCH_BUDGET_DEAD_END_HEURISTIC_H_

#include "heuristic.h"
#include <iostream>


class BudgetDeadEndHeuristic: public Heuristic {
	Heuristic *budget_heuristic;
	Heuristic *prob_cost_heuristic;
protected:
	virtual void initialize();
	virtual int compute_heuristic(const GlobalState&) {std::cout << "compute_heuristic() without budget not supported here!" << std::endl; exit(-1);}
	virtual int compute_heuristic(const GlobalState &state, int budget);
public:
	Heuristic* get_prob_cost_heuristic() {return prob_cost_heuristic; }
	BudgetDeadEndHeuristic(const Options &opts);
	virtual ~BudgetDeadEndHeuristic();
};


#endif /* SRC_SEARCH_BUDGET_DEAD_END_HEURISTIC_H_ */
