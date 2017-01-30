/*
 * budget_dead_end_heuristic.cpp
 *
 *  Created on: 03.01.2017
 *      Author: Patrick
 */

#include "budget_dead_end_heuristic.h"
#include "option_parser.h"
#include "plugin.h"

using namespace std;

BudgetDeadEndHeuristic::BudgetDeadEndHeuristic(const Options &opts)
: Heuristic(opts) {
	needs_buget = true;
	budget_heuristic = opts.get<Heuristic*>("budget_heuristic");
	prob_cost_heuristic = opts.get<Heuristic*>("prob_cost_heuristic");
}

BudgetDeadEndHeuristic::~BudgetDeadEndHeuristic() {

}

void BudgetDeadEndHeuristic::initialize() {
    cout << "Initializing BudgetDeadEndHeuristic..." << endl;
}

int BudgetDeadEndHeuristic::compute_heuristic(const GlobalState &state, int budget) {
	if (test_goal(state))
		return 0;

	budget_heuristic->evaluate(state);
	int budget_heuristic_value = budget_heuristic->get_value();
	if(budget_heuristic_value == DEAD_END || budget_heuristic_value > budget) {
		//cout << "Actually detected DEAD_END!" << endl;
		return DEAD_END;
	}

	prob_cost_heuristic->evaluate(state, budget);
	int prob_cost_heuristic_value = prob_cost_heuristic->get_value();

	return prob_cost_heuristic_value;
}

void BudgetDeadEndHeuristic::reinitialize(AttackSearchSpace* attack_search_space, SearchSpace* search_space, OpenList<pair<StateID, int>>* open_list, GlobalState goal_state, int goal_state_budget)  {
	prob_cost_heuristic-> reinitialize(attack_search_space, search_space, open_list, goal_state, goal_state_budget);
}
void BudgetDeadEndHeuristic::set_curr_attack_search_space (AttackSearchSpace* attack_search_space) {
	prob_cost_heuristic->set_curr_attack_search_space(attack_search_space);
}
AttackSearchSpace* BudgetDeadEndHeuristic::get_curr_attack_search_space () {
	return prob_cost_heuristic->get_curr_attack_search_space();
}
void BudgetDeadEndHeuristic::reset () {
	prob_cost_heuristic->reset();
	budget_heuristic->reset();
}

static Heuristic *_parse(OptionParser &parser) {
  /*  parser.document_synopsis("Blind heuristic",
                             "Returns cost of cheapest action for "
                             "non-goal states, "
                             "0 for goal states");
    parser.document_language_support("action costs", "supported");
    parser.document_language_support("conditional effects", "supported");
    parser.document_language_support("axioms", "supported");
    parser.document_property("admissible", "yes");
    parser.document_property("consistent", "yes");
    parser.document_property("safe", "yes");
    parser.document_property("preferred operators", "no");*/

	parser.add_option<Heuristic*>("budget_heuristic");
	parser.add_option<Heuristic*>("prob_cost_heuristic");
    Heuristic::add_options_to_parser(parser);

    Options opts = parser.parse();
    if (parser.dry_run())
        return 0;
    else
        return new BudgetDeadEndHeuristic(opts);
}

static Plugin<Heuristic> _plugin("budget_dead_end", _parse);

