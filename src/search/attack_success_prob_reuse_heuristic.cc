/*
 * attack_success_prob_reuse_heuristic.cc
 *
 *  Created on: 20.12.2016
 *      Author: Patrick
 */

#include "attack_success_prob_reuse_heuristic.h"
#include "global_operator.h"
#include "global_state.h"
#include "globals.h"
#include "option_parser.h"
#include "plugin.h"

#include <limits>
#include <utility>

AttackSuccessProbReuseHeuristic::AttackSuccessProbReuseHeuristic(const Options &opts)
: Heuristic(opts) {
}


AttackSuccessProbReuseHeuristic::~AttackSuccessProbReuseHeuristic() {
}

void AttackSuccessProbReuseHeuristic::initialize() {
    cout << "Initializing AttackSuccessProbReuse heuristic..." << endl;
}

int AttackSuccessProbReuseHeuristic::compute_heuristic(const GlobalState &state) {
    if (test_goal(state))
        return 0;
    else
    	return 0;
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

    Heuristic::add_options_to_parser(parser);
    Options opts = parser.parse();
    if (parser.dry_run())
        return 0;
    else
        return new AttackSuccessProbReuseHeuristic(opts);
}

static Plugin<Heuristic> _plugin("attack_success_prob_reuse", _parse);

