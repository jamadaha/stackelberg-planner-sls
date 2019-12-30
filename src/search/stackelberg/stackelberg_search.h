/*
 * stackelberg_search is a copy of fix action search to introduce modifications in the way
 * that we call the search engine so that it is compatible with symbolic search engines.
 * 
 * I'm also removing optional stuff that is currently unnecessary here to simplify the
 * code and understand it better. We can re-add those options later on.
 */

#ifndef STACKELBERG_SEARCH_H
#define STACKELBERG_SEARCH_H

#include <unordered_set>
#include <limits>
#include <memory>

#include "pareto_frontier.h"
#include "stackelberg_task.h"

#include "../search_engine.h"
#include "../successor_generator.h"
#include "../per_fix_state_information.h"

namespace stackelberg {
    
    struct FixSearchInfoAttackPlan {
        int follower_plan_prob_cost;
        std::vector<int> follower_plan;
    FixSearchInfoAttackPlan(int _follower_plan_prob_cost = -1, std::vector<int> _follower_plan = std::vector<int>())
    : follower_plan_prob_cost(_follower_plan_prob_cost),
            follower_plan(_follower_plan){ }
    };

    struct FixSearchInfoFixSequence {
        int leader_actions_cost;
        bool already_in_frontier;
    FixSearchInfoFixSequence(int _leader_actions_cost = -1, bool _already_in_frontier = false)
    : leader_actions_cost(_leader_actions_cost),
            already_in_frontier(_already_in_frontier){ }
    };

    class PartialOrderReduction;


    class StackelbergSearch: public SearchEngine {
    private:
        const double follower_budget_factor;
        const double leader_budget_factor;
    
        const bool use_partial_order_reduction;
        const bool check_parent_follower_plan_applicable;
        const bool check_leader_state_already_known;
        const bool do_follower_op_dom_pruning;
        const bool use_IDS;
        //disabled: const bool sort_leader_ops_stupid;
        const bool sort_leader_ops_advanced;
        // disabled: const bool greedy_leader_search;
        const bool upper_bound_pruning;

        std::unique_ptr<StackelbergTask> task;

        std::unique_ptr<PartialOrderReduction> por;
        
        std::unique_ptr<SuccessorGeneratorSwitch> leader_operators_successor_generator;

	std::unique_ptr<StateRegistry> leader_vars_state_registry;

	std::vector<std::vector<int>> dominated_follower_op_ids;

	SearchEngine* search_engine;
	Heuristic* follower_heuristic;

	ParetoFrontier pareto_frontier;
        
	PerFixStateInformation<FixSearchInfoAttackPlan> leader_search_node_infos_follower_plan;
	PerStateInformation<FixSearchInfoFixSequence> leader_search_node_infos_leader_sequence;
	int max_leader_actions_budget = UNLTD_BUDGET;
	int curr_leader_actions_budget = UNLTD_BUDGET;

	bool returned_somewhere_bc_of_budget = false;

	double ids_leader_budget_factor = 1.5;

	int follower_cost_upper_bound;
	int leader_action_costs_for_follower_upper_bound = std::numeric_limits<int>::max();
        
	int num_recursive_calls = 0;
	int num_follower_searches = 0;
	long follower_search_duration_sum = 0;
	long reset_and_initialize_duration_sum = 0;
	long leader_search_initialize_duration = 0;
	int all_follower_states = 0;
	int spared_follower_searches_because_leader_state_already_seen = 0;
	int spared_follower_searches_because_parent_plan_applicable = 0;
	int num_leader_op_paths = 0;
	int num_recursive_calls_for_sorting = 0;

	void iterate_applicable_ops(const std::vector<const GlobalOperator*>& applicable_ops_after_pruning, const GlobalState& state, const std::vector<int> &follower_plan, int follower_plan_cost, std::vector<const GlobalOperator*>& leader_ops_sequence, std::vector<int>& sleep, AttackSearchSpace* follower_heuristic_search_space, bool recurse, std::vector<int> &recursive_follower_costs);

    protected:
        virtual void initialize();
        virtual SearchStatus step();
                
        int compute_pareto_frontier(const GlobalState &state,
                                    std::vector<const GlobalOperator*> &leader_ops_sequence,
                                    int leader_actions_cost,
                                    const std::vector<int> &parent_follower_plan,
                                    int parent_follower_plan_cost,
                                    std::vector<int> &sleep, bool recurse);
        
    public:
        StackelbergSearch(const Options &opts);
        virtual ~StackelbergSearch() = default;
    };

}
#endif /* STACKELBERG_SEARCH_H */
