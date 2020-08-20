#include "search_engine.h"

#include "countdown_timer.h"
#include "globals.h"
#include "operator_cost.h"
#include "option_parser.h"
#include "stackelberg/plan_reuse.h"

#include <cassert>
#include <iostream>
#include <limits>

using namespace std;


SearchEngine::SearchEngine(const Options &opts)
    : status(IN_PROGRESS),
      current_plan_cost(std::numeric_limits<int>::max()), solution_found(false), 
      search_space(OperatorCost(opts.get_enum("cost_type"))),
      cost_type(OperatorCost(opts.get_enum("cost_type"))),
      max_time(opts.get<double>("max_time"))
{
    if (opts.get<int>("bound") < 0) {
        cerr << "error: negative cost bound " << opts.get<int>("bound") << endl;
        exit_with(EXIT_INPUT_ERROR);
    }
    bound = opts.get<int>("bound");
}

SearchEngine::~SearchEngine()
{
}

void SearchEngine::statistics() const
{
}

bool SearchEngine::found_solution() const
{
    return solution_found;
}

SearchStatus SearchEngine::get_status() const
{
    return status;
}

const SearchEngine::Plan &SearchEngine::get_plan() const
{
    assert(solution_found);
    return plan;
}

void SearchEngine::set_plan(const Plan &p)
{
    solution_found = true;
    plan = p;
    current_plan_cost = ::calculate_plan_cost(plan);

    cout << "plan length: " << plan.size() << endl;
    cout << "current plan cost: " << current_plan_cost << endl;
}

void SearchEngine::search(double max_time_)
{
    solution_found = false;
    status = IN_PROGRESS;
    initialize();

    if (max_time_ > 0) {
        max_time = max_time_;
    }

    CountdownTimer timer(max_time);
    while (status == IN_PROGRESS) {
        status = step();
        if (timer.is_expired()) {
            cout << "Time limit reached. Abort search." << endl;
            status = TIMEOUT;
            break;
        }
    }
    cout << "Actual search time: " << timer
         << " [t=" << g_timer << "]" << endl;
}

bool SearchEngine::check_goal_and_set_plan(const GlobalState &state,int budget)
{
    if (test_goal(state)) {
        cout << "Solution found!" << endl;
        Plan plan;
        search_space.trace_path(state, plan, budget);
        set_plan(plan);
        goal_state.reset(new GlobalState(state));
        goal_state_budget = budget;
        return true;
    }
    return false;
}

bool SearchEngine::check_goal_and_set_plan_generation(const GlobalState &state,int budget, int g)
{
    if (!opposite_frontier) {
        return false;
    }
    
    int goal_cost = opposite_frontier->check_goal_cost(state);
    if (goal_cost < std::numeric_limits<int>::max() && goal_cost + g < current_plan_cost) {     
        cout << "Solution found by opposite frontier!" << endl;
        cout << "g value: " << g << endl;
        cout << "goal cost: " << goal_cost << endl;
        Plan plan;
        search_space.trace_path(state, plan, budget);
        opposite_frontier->getPlan(state, goal_cost, plan);
        set_plan(plan);
        goal_state.reset(new GlobalState(state));
        goal_state_budget = budget;

        cout << "Checking " << current_plan_cost << " against a desired bound of " << opposite_frontier->get_desired_bound() << endl;
        return current_plan_cost <= opposite_frontier->get_desired_bound();
 //TODO: compare againstlower bound
    }
    return false;
}

void SearchEngine::save_plan_if_necessary()
{
    if (found_solution()) {
        save_plan(get_plan(), 0);
    }
}

int SearchEngine::get_adjusted_cost(const GlobalOperator &op) const
{
    return get_adjusted_action_cost(op, cost_type);
}

int SearchEngine::calculate_plan_cost() const
{
    return current_plan_cost;
}

void SearchEngine::add_options_to_parser(OptionParser &parser)
{
    ::add_cost_type_option_to_parser(parser);
    parser.add_option<int>(
        "bound",
        "exclusive depth bound on g-values. Cutoffs are always performed according to "
        "the real cost, regardless of the cost_type parameter", "infinity");
    parser.add_option<double>(
        "max_time",
        "maximum time in seconds the search is allowed to run for. The "
        "timeout is only checked after each complete search step "
        "(usually a node expansion), so the actual runtime can be arbitrarily "
        "longer. Therefore, this parameter should not be used for time-limiting "
        "experiments. Timed-out searches are treated as failed searches, "
        "just like incomplete search algorithms that exhaust their search space.",
        "infinity");
}
