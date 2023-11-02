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
: Heuristic(opts),
  curr_attack_search_space(NULL){
	needs_buget = true;
	default_heuristic = opts.get<Heuristic*>("default_heuristic");
}


AttackSuccessProbReuseHeuristic::~AttackSuccessProbReuseHeuristic() {
}

void AttackSuccessProbReuseHeuristic::initialize() {
    cout << "Initializing AttackSuccessProbReuse heuristic..." << endl;
}

AttackSearchInfo& AttackSuccessProbReuseHeuristic::get_attack_search_info (const GlobalState &state, int budget) {
	BudgetSearchNodeInfo &budget_search_node_info = curr_attack_search_space->budget_attack_search_node_infos[state];
	int index;

	if(budget_search_node_info.search_node_info_for_budget.find(budget) != budget_search_node_info.search_node_info_for_budget.end()) {
		index = budget_search_node_info.search_node_info_for_budget[budget];
	} else {
		curr_attack_search_space->all_attack_search_node_infos.push_back(AttackSearchInfo());
		index = (int) curr_attack_search_space->all_attack_search_node_infos.size() - 1;
		budget_search_node_info.search_node_info_for_budget[budget] = index;
	}

	return curr_attack_search_space->all_attack_search_node_infos[index];
}

void AttackSuccessProbReuseHeuristic::reinitialize(AttackSearchSpace* attack_search_space, SearchSpace* search_space, OpenList<pair<StateID, int>>* open_list, GlobalState goal_state, int goal_state_budget) {
	//curr_per_state_information = per_state_information;
	curr_attack_search_space = attack_search_space;
	vector<pair<GlobalState, int>> own_open_list;

	AttackSearchInfo &goal_info = get_attack_search_info(goal_state, goal_state_budget);
	goal_info.attack_plan_prob_cost_heuristic_value = 0;
	own_open_list.push_back(pair<GlobalState, int>(goal_state, goal_state_budget));

	while (!open_list->empty()) {
		pair<StateID, int> state_id_and_budget = open_list->remove_min(NULL);
        StateID id = state_id_and_budget.first;
        int budget = state_id_and_budget.second;
        const GlobalState &state = g_state_registry->lookup_state(id);
        const SearchNode &node = search_space->get_node(state, budget);
        AttackSearchInfo &info = get_attack_search_info(state, budget);
    		info.attack_plan_prob_cost_heuristic_value = node.get_h();
    		own_open_list.push_back(pair<GlobalState, int>(state, budget));
	}


	while (!own_open_list.empty()) {
		GlobalState current_state = own_open_list.back().first;
		int current_budget = own_open_list.back().second;
		own_open_list.pop_back();

		const SearchNode &current_node = search_space->get_node(current_state, current_budget);

		const std::vector<pair<StateID, int>> & all_parent_state_ids = current_node.get_all_parent_state_ids();
		const std::vector<const GlobalOperator*> &all_parent_creating_operators =
				current_node.get_all_parent_creating_operators();

		AttackSearchInfo &node_info = get_attack_search_info(current_state, current_budget);

		for (size_t state_no = 0; state_no < all_parent_state_ids.size(); state_no++) {
			const GlobalState &parent_state = g_state_registry->lookup_state(all_parent_state_ids[state_no].first);
			int parent_budget = all_parent_state_ids[state_no].second;
			SearchNode parent_node = search_space->get_node(parent_state, parent_budget);
			AttackSearchInfo &parent_info = get_attack_search_info(parent_state, parent_budget);
			int temp_attack_plan_prob_cost_heuristic_value = node_info.attack_plan_prob_cost_heuristic_value
					+ get_adjusted_cost(*all_parent_creating_operators[state_no]);
			if (parent_info.attack_plan_prob_cost_heuristic_value > temp_attack_plan_prob_cost_heuristic_value) {
				parent_info.attack_plan_prob_cost_heuristic_value = temp_attack_plan_prob_cost_heuristic_value;
			}
			parent_node.decrement_child_num();
			if(parent_node.get_child_num() == 0) {
				own_open_list.push_back(pair<GlobalState, int>(parent_state, parent_budget));
			}
		}
	}
}

int AttackSuccessProbReuseHeuristic::compute_heuristic(const GlobalState &state, int budget) {
	if (test_goal(state))
		return 0;

	if(curr_attack_search_space == NULL) {
		// return heuristic value computed by default heuristic
		default_heuristic->evaluate(state);
		return default_heuristic->get_value();
	}

	AttackSearchInfo &info = get_attack_search_info(state, budget);
	int attack_plan_prob_cost_heuristic_value = info.attack_plan_prob_cost_heuristic_value;
	if (attack_plan_prob_cost_heuristic_value == numeric_limits<int>::max()) {
		// return heuristic value computed by default heuristic
		default_heuristic->evaluate(state);
		return default_heuristic->get_value();
	}

	return attack_plan_prob_cost_heuristic_value;
}

void AttackSuccessProbReuseHeuristic::reset() {
	default_heuristic->reset();
}

static Heuristic *_parse(OptionParser &parser) {

    parser.add_option<Heuristic*>("default_heuristic");
    Heuristic::add_options_to_parser(parser);

    Options opts = parser.parse();
    if (parser.dry_run())
        return 0;
    else
        return new AttackSuccessProbReuseHeuristic(opts);
}

static Plugin<Heuristic> _plugin("attack_success_prob_reuse", _parse);

