#ifndef STACKELBERG_BOUNDED_H
#define STACKELBERG_BOUNDED_H

#include <unordered_set>
#include <limits>
#include <memory>

#include "pareto_frontier.h"
#include "stackelberg_task.h"



#include "../g_evaluator.h"

#include "../search_engine.h"
#include "../successor_generator.h"
#include "../attack_success_prob_reuse_heuristic.h"
#include "../per_fix_state_information.h"

typedef std::pair<StateID, const GlobalOperator *> OpenListEntryLazy;

namespace stackelberg {

    class FollowerSearchEngine;
    
    struct LeaderSearchNode {
        int leader_actions_cost;
        bool already_in_frontier;
    LeaderSearchNode(int _leader_actions_cost = -1, bool _already_in_frontier = false)
    : leader_actions_cost(_leader_actions_cost),
            already_in_frontier(_already_in_frontier){ }
    };

    class PartialOrderReduction;

    class StackelbergBounded : public SearchEngine {
    protected:
        const bool use_partial_order_reduction;

        std::unique_ptr<StackelbergTask> task;
        std::unique_ptr<PartialOrderReduction> por;
        std::unique_ptr<SuccessorGeneratorSwitch> leader_operators_successor_generator;
	std::unique_ptr<StateRegistry> leader_vars_state_registry;

        std::unique_ptr<FollowerSearchEngine> optimal_engine;
        std::unique_ptr<FollowerSearchEngine> cost_bounded_engine;
        std::unique_ptr<FollowerSearchEngine> plan_repair;
        
	ParetoFrontier pareto_frontier;
        
	PerFixStateInformation<FollowerTask> follower_task_info;

        PerStateInformation<LeaderSearchNode> leader_search_node_info;        

        int num_follower_searches = 0;
        int num_follower_searches_optimal = 0;
        int num_follower_searches_cost_bounded = 0;
        int num_follower_searches_plan_repair = 0;
        
        long follower_search_duration_sum = 0;
	long reset_and_initialize_duration_sum = 0;
	long leader_search_initialize_duration = 0;
	int all_follower_states = 0;
	int num_leader_op_paths = 0;

    protected:
        virtual void initialize();
        virtual SearchStatus step();

        virtual std::vector<FollowerTask *> gather_follower_tasks(int leader_cost, int follower_cost_bound) = 0;  

        virtual int get_next_L() = 0;
    public:
        StackelbergBounded(const Options &opts);
        virtual ~StackelbergBounded() = default;
    };


    class StackelbergBoundedBFS : public StackelbergBounded {
        std::unique_ptr<GlobalState> current_state;
        StateID current_predecessor_id;
        const GlobalOperator *current_operator;
        int current_g;

        GEvaluator g_evaluator;
        std::unique_ptr<OpenList<OpenListEntryLazy>> open_list;

    public:
        StackelbergBoundedBFS(const Options &opts);
        virtual ~StackelbergBoundedBFS() = default;


        virtual void initialize() override;
        virtual std::vector<FollowerTask *> gather_follower_tasks(int leader_cost, int follower_cost_bound) override;

        virtual int get_next_L() override {
            return current_g;
        }
    }; 

}
#endif /* STACKELBERG_BOUNDED_H */
