#ifndef SEARCH_STATE_EXPLORER_H
#define SEARCH_STATE_EXPLORER_H

#include <filesystem>
#include <limits>
#include <memory>
#include <unordered_set>

#include "../stackelberg_statistics.h"

#include "../pareto_frontier.h"
#include "../stackelberg_task.h"

#include "../../attack_success_prob_reuse_heuristic.h"
#include "../../search_engine.h"
#include "../../successor_generator.h"
#include "../../symbolic/sym_enums.h"
#include "../../symbolic/sym_state_space_manager.h"

#include "../../symbolic/sym_params_search.h"
#include "world.h"


namespace symbolic {
    class UniformCostSearch;
    class SymVariables;
    class SymController;
} // namespace symbolic

namespace stackelberg {
    class PlanReuse;
    class FollowerSearchEngine;
    class SymbolicStackelbergManager;
    class StateExplorer : public SearchEngine {
    public:
        explicit StateExplorer(const Options &opts);
        ~StateExplorer() override = default;

    protected:
        std::unique_ptr<StackelbergTask> task;
        std::shared_ptr<SymbolicStackelbergManager> stackelberg_mgr;
        std::unique_ptr<FollowerSearchEngine> optimal_engine;
        std::unique_ptr<FollowerSearchEngine> cost_bounded_engine;
        std::unique_ptr<PlanReuse> plan_reuse;
        std::shared_ptr<symbolic::SymVariables> vars;
        std::unique_ptr<symbolic::SymController> leader_search_controller;
        std::unique_ptr<symbolic::UniformCostSearch> leader_search;
        symbolic::SymParamsMgr
                mgrParams; // Parameters for SymStateSpaceManager configuration.
        symbolic::SymParamsSearch
                searchParams; // Parameters to search the original state space
        ParetoFrontier pareto_frontier;
        World world;

        const size_t min_precondition_size;
        const size_t max_precondition_size;

        void initialize() final;
        SearchStatus step() final;
    };
}
#endif //SEARCH_STATE_EXPLORER_H