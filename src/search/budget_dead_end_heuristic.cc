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
	budget_heuristic = opts.get<Heuristic*>("budget_heuristic");
	prob_cost_heuristic = opts.get<Heuristic*>("prob_cost_heuristic");
}

BudgetDeadEndHeuristic::~BudgetDeadEndHeuristic() {

}

void BudgetDeadEndHeuristic::initialize() {
    cout << "Initializing AttackSuccessProbReuse heuristic..." << endl;
}

int BudgetDeadEndHeuristic::compute_heuristic(const GlobalState &state, int budget) {
	if (test_goal(state))
		return 0;

	return budget;
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

