#ifndef SEARCH_ENGINE_H
#define SEARCH_ENGINE_H

#include <vector>
#include <memory>

class Heuristic;
class OptionParser;
class Options;

#include "global_operator.h"
#include "operator_cost.h"
#include "search_space.h"
#include "search_progress.h"
#include "open_lists/open_list.h"

enum SearchStatus {IN_PROGRESS, TIMEOUT, FAILED, SOLVED};

namespace stackelberg {
    class OppositeFrontierExplicit;
}

class SearchEngine
{
public:
    typedef std::vector<const GlobalOperator *> Plan;
private:
    SearchStatus status;
    Plan plan;
    int current_plan_cost;
protected:
    bool solution_found;
    SearchSpace search_space;
    SearchProgress search_progress;
    int bound;
    OperatorCost cost_type;
    double max_time;

    std::unique_ptr<GlobalState> goal_state;
    int goal_state_budget = UNLTD_BUDGET;

    std::shared_ptr<stackelberg::OppositeFrontierExplicit> opposite_frontier;

    virtual void initialize() {}
    virtual SearchStatus step() = 0;

    void set_plan(const Plan &plan);
    bool check_goal_and_set_plan(const GlobalState &state,
                                 int budget = UNLTD_BUDGET);
    bool check_goal_and_set_plan_generation(const GlobalState &state, int budget, int g);

    int get_adjusted_cost(const GlobalOperator &op) const;
public:
    SearchEngine(const Options &opts);
    virtual ~SearchEngine();
    virtual void statistics() const;
    virtual void heuristic_statistics() const {}
    virtual void save_plan_if_necessary();
    bool found_solution() const;
    SearchStatus get_status() const;
    const Plan &get_plan() const;
    void search();
    SearchProgress get_search_progress() const
    {
        return search_progress;
    }
    void set_bound(int b)
    {
        bound = b;
    }
    int get_bound()
    {
        return bound;
    }
    virtual void reset() {
        solution_found = false;
        plan.clear();
        current_plan_cost = std::numeric_limits<int>::max();
    };
    SearchSpace *get_search_space()
    {
        return &search_space;
    }
    const GlobalState *get_goal_state()
    {
        return goal_state.get();
    }
    int get_goal_state_budget()
    {
        return goal_state_budget;
    }

     void set_opposite_frontier(std::shared_ptr<stackelberg::OppositeFrontierExplicit> opposite_frontier_) {
         opposite_frontier = opposite_frontier_;
     }

    static void add_options_to_parser(OptionParser &parser);

    virtual int calculate_plan_cost() const;
};

#endif
