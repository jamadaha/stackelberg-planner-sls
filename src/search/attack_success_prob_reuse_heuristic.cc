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

void AttackSuccessProbReuseHeuristic::reinitialize(PerStateInformation<AttackSearchInfo> &per_state_information, SearchSpace* search_space, OpenList<StateID>* open_list, const GlobalState* goal_state) {
	vector<const GlobalState *> own_open_list;

	AttackSearchInfo &goal_info = per_state_information[*goal_state];
	goal_info.attack_plan_prob_cost_heuristic_value = 0;
	own_open_list.push_back(goal_state);

	while (!open_list->empty()) {
        StateID id = open_list->remove_min(NULL);
        const GlobalState &state = g_state_registry->lookup_state(id);
        const SearchNode &node = search_space->get_node(state);
    	AttackSearchInfo &info = per_state_information[state];
    	info.attack_plan_prob_cost_heuristic_value = node.get_h();
    	own_open_list.push_back(&state);
	}


	while (!own_open_list.empty()) {
		const GlobalState *current_state = own_open_list.back();
		own_open_list.pop_back();

		const SearchNode &current_node = search_space->get_node(*current_state);

		const std::vector<StateID> & all_parent_state_ids = current_node.get_all_parent_state_ids();
		const std::vector<const GlobalOperator*> &all_parent_creating_operators =
				current_node.get_all_parent_creating_operators();

		AttackSearchInfo &node_info = per_state_information[*current_state];

		for (size_t state_no = 0; state_no < all_parent_state_ids.size(); state_no++) {
			const GlobalState &parent_state = g_state_registry->lookup_state(all_parent_state_ids[state_no]);
			SearchNode &parent_node = search_space->get_node(parent_state);
			AttackSearchInfo &parent_info = per_state_information[parent_state];
			int temp_attack_plan_prob_cost_heuristic_value = node_info.attack_plan_prob_cost_heuristic_value
					+ get_adjusted_cost(*all_parent_creating_operators[state_no]);
			if (parent_info.attack_plan_prob_cost_heuristic_value > temp_attack_plan_prob_cost_heuristic_value) {
				parent_info.attack_plan_prob_cost_heuristic_value = temp_attack_plan_prob_cost_heuristic_value;
			}
			parent_node.decrement_child_num();
			if(parent_node.get_child_num() == 0) {
				own_open_list.push_back(&parent_state);
			}
		}
	}
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

