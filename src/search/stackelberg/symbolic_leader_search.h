#ifndef STACKELBERG_BOUNDED_H
#define STACKELBERG_BOUNDED_H

#include <unordered_set>
#include <limits>
#include <memory>

#include "stackelberg_statistics.h"

#include "pareto_frontier.h"
#include "stackelberg_task.h"

#include "../search_engine.h"
#include "../successor_generator.h"
#include "../attack_success_prob_reuse_heuristic.h"
#include "../symbolic/sym_enums.h"
#include "../symbolic/sym_state_space_manager.h"

#include "../symbolic/sym_params_search.h"

namespace symbolic {
    class UniformCostSearch;
    class SymVariables;
    class SymController;
}

namespace stackelberg {

    class PlanReuse;
    class FollowerSearchEngine;
    class SymbolicStackelbergManager;

    class SymbolicStackelberg : public SearchEngine {
    protected:
        std::unique_ptr<StackelbergTask> task;        
        std::shared_ptr<SymbolicStackelbergManager> stackelberg_mgr;

        std::unique_ptr<FollowerSearchEngine> optimal_engine;
        std::unique_ptr<FollowerSearchEngine> satisficing_engine;

        std::unique_ptr<PlanReuse> plan_reuse;

        std::shared_ptr<symbolic::SymVariables> vars;
        std::unique_ptr<symbolic::SymController> leader_search_controller;
        std::unique_ptr<symbolic::UniformCostSearch> leader_search;
        symbolic::SymParamsMgr mgrParams; //Parameters for SymStateSpaceManager configuration.
        symbolic::SymParamsSearch searchParams; //Parameters to search the original state space

        const bool upper_bound_pruning;
        
	ParetoFrontier pareto_frontier;

        StackelbergStatistics statistics;

    protected:    
        virtual void initialize() override;
        virtual SearchStatus step() override;
    public:
        SymbolicStackelberg(const Options &opts);
        virtual ~SymbolicStackelberg() = default;
    };


}
#endif 
