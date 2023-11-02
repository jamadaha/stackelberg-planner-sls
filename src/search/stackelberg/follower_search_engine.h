#ifndef STACKELBERG_FOLLOWER_SEARCH_ENGINE_H
#define STACKELBERG_FOLLOWER_SEARCH_ENGINE_H

#include "../symbolic/sym_enums.h"
#include "../symbolic/sym_state_space_manager.h"
#include "../symbolic/sym_solution.h"
#include "../symbolic/sym_params_search.h"
#include "../search_progress.h"

#include <memory>

namespace symbolic {
    class SymVariables;
    class SymSolution;
    class ClosedList;
}

class SearchEngine;
class Heuristic;
class Options;
class OptionParser;
namespace stackelberg {


    class FollowerSolution {
        bool solved;
        int plan_cost;
        int lower_bound;

        std::vector<int> initial_state;
        std::vector <const GlobalOperator *> plan;


        // If symbolic bidirectional search was used as a sub-solver, then we have both
        // frontiers, which can be used for plan reconstruction strategies
        std::shared_ptr<symbolic::ClosedList> closed_fw, closed_bw;
        BDD cut;
        int cut_cost;
        std::map<int, std::vector<symbolic::TransitionRelation>> trs;
        

    public:
        
    FollowerSolution() :
        solved(false), plan_cost(-1), lower_bound(-1) {
        }

        FollowerSolution (int cost, const std::vector<int> & initial_state_, const std::vector <const GlobalOperator *> & plan_,
                          int lb = 0) :
            solved(true), plan_cost (cost), lower_bound(lb), initial_state(initial_state_), plan(plan_)  {
        }

        FollowerSolution (int cost, int lb = 0) : solved(true), plan_cost (cost), lower_bound(lb) {}


        //Constructor for symbolic search solver
        FollowerSolution (const symbolic::SymSolution & sol,
                          const std::vector<int> & leader_state,
                          const std::vector<bool> & pattern,
                          int lb, std::shared_ptr<symbolic::ClosedList> closed_fw,
                          std::shared_ptr<symbolic::ClosedList> closed_bw);
 

        int solution_cost() const{
            return plan_cost;
        }

        int get_lower_bound() const {
            return lower_bound;
        }

        bool is_optimal () const {
            return plan_cost == lower_bound || lower_bound == std::numeric_limits<int>::max();
        }
        
        bool is_solved() const {
            return solved;
        }

       
        bool has_plan() const {
            return !plan.empty();
        }            

        const std::vector <const GlobalOperator *>  & get_plan() const{
            return plan;
        }

        std::shared_ptr<symbolic::ClosedList>  get_closed_fw() const {
            return closed_fw;
        }
        
        std::shared_ptr<symbolic::ClosedList>  get_closed_bw() const {
            return closed_bw;
        }

        BDD getCut() const {
            return cut;
        }

        int getCutCost() const {
            return cut_cost;
        }

        const std::map<int, std::vector<symbolic::TransitionRelation>> get_transition_relation() const {
            return trs;
        }

        const std::vector<int> & get_initial_state () const {
            return initial_state;
        }

    };


    class PlanReuse;
    
    class FollowerTask;
    class StackelbergTask;

    class SymbolicStackelbergManager;

    class FollowerSearchEngine {
    protected:
        StackelbergTask * task;
        std::shared_ptr<SymbolicStackelbergManager> stackelberg_mgr;
        
        const bool plan_reuse_upper_bound;

        const int time_limit_seconds_minimum_task;

        virtual void initialize_follower_search_engine() = 0; 
            
    public:
        FollowerSearchEngine(const Options &opts);

        virtual ~FollowerSearchEngine() = default;

        void initialize(StackelbergTask * _task, std::shared_ptr<SymbolicStackelbergManager> mgr) {
            task = _task;
            stackelberg_mgr = mgr;
            initialize_follower_search_engine();
        }
        
        //Desired bound means: you can return as soon as you have a solution of this
        //quality or less. But the method should return the cost of the best solution
        //found independently of whether it is larger or lower than desired bound
        virtual FollowerSolution solve (const std::vector<int> & leader_state, PlanReuse * plan_reuse, int bound) = 0;

        virtual FollowerSolution solve_minimum_ftask (PlanReuse * plan_reuse) = 0;
        
        static void add_options_to_parser(OptionParser &parser);

        virtual void print_statistics (){}

    };

    class ExplicitFollowerSearchEngine : public FollowerSearchEngine {
        SearchEngine* search_engine;
        SearchEngine* search_engine_up;

        std::unique_ptr<SuccessorGenerator> successor_generator;
        std::vector <GlobalOperator> follower_operators_with_all_preconds;

        SearchProgress follower_statistics;

        const bool is_optimal_solver;
        const bool debug;
        
        virtual void initialize_follower_search_engine() override; 

    public:
        ExplicitFollowerSearchEngine(const Options &opts);
        virtual FollowerSolution solve (const std::vector<int> & leader_state, PlanReuse * plan_reuse, int bound) override;
        virtual FollowerSolution solve_minimum_ftask (PlanReuse * plan_reuse) override;

        virtual void print_statistics () override {
            follower_statistics.print_statistics();
        }
    };

    class SymbolicFollowerSearchEngine : public FollowerSearchEngine {
        std::shared_ptr<symbolic::SymVariables> vars;
        
        symbolic::SymParamsMgr mgrParams; //Parameters for SymStateSpaceManager configuration.
        symbolic::SymParamsSearch searchParams; //Parameters to search the original state space

        const bool bidir;
        const bool plan_reuse_minimal_task_upper_bound;
        const int force_bw_search_minimum_task_seconds;
        int force_bw_search_first_task_seconds;
        const bool store_lower_bound;
        
        virtual void initialize_follower_search_engine() override;
        
    public:
        SymbolicFollowerSearchEngine(const Options &opts);
        virtual FollowerSolution solve (const std::vector<int> & leader_state, PlanReuse * plan_reuse, int bound) override;
        virtual FollowerSolution solve_minimum_ftask (PlanReuse * plan_reuse) override;
    }; 

}








#endif
