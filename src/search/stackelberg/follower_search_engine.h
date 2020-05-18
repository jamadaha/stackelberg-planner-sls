#ifndef STACKELBERG_FOLLOWER_SEARCH_ENGINE_H
#define STACKELBERG_FOLLOWER_SEARCH_ENGINE_H

#include "../symbolic/sym_enums.h"
#include "../symbolic/sym_state_space_manager.h"
#include "../symbolic/sym_solution.h"
#include "../symbolic/sym_params_search.h"

#include <memory>

namespace symbolic {
    class SymVariables;
    class SymSolution;
}

class SearchEngine;
class Heuristic;
class Options;
namespace stackelberg {


    class FollowerSolution {
        bool solved;
        
        int plan_cost;
        std::vector <const GlobalOperator *> plan;

    public:
        
    FollowerSolution() :
        solved(false), plan_cost(-1) {
        }

        FollowerSolution (int cost) : solved(true), plan_cost (cost) {}

        FollowerSolution (const symbolic::SymSolution & sol,
                          const std::vector<int> & leader_state,
                          const std::vector<bool> & pattern);

        int solution_cost() const{
            return plan_cost;
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
    };

    class PlanReuse;
    
    class FollowerTask;
    class StackelbergTask;

    class SymbolicStackelbergManager;

    class FollowerSearchEngine {
    protected:
        StackelbergTask * task;
        
        std::shared_ptr<SymbolicStackelbergManager> stackelberg_mgr;

        virtual void initialize_follower_search_engine() = 0; 
            
    public:
        void initialize(StackelbergTask * _task, std::shared_ptr<SymbolicStackelbergManager> mgr) {
            task = _task;
            stackelberg_mgr = mgr;
            initialize_follower_search_engine();
        }
        
        //Desired bound means: you can return as soon as you have a solution of this
        //quality or less. But the method should return the cost of the best solution
        //found independently of whether it is larger or lower than desired bound
        virtual FollowerSolution solve (const std::vector<int> & leader_state, PlanReuse * plan_reuse = nullptr) = 0;

        virtual FollowerSolution solve_minimum_ftask () = 0;        
    };

    class ExplicitFollowerSearchEngine : public FollowerSearchEngine {
        SearchEngine* search_engine;
        Heuristic * follower_heuristic; //We need a pointer to the heuristic to reset it!

        virtual void initialize_follower_search_engine() override; 

    public:
        ExplicitFollowerSearchEngine(const Options &opts);
        virtual FollowerSolution solve (const std::vector<int> & leader_state, PlanReuse * plan_reuse = nullptr) override;
        virtual FollowerSolution solve_minimum_ftask () override;
    };

    class SymbolicFollowerSearchEngine : public FollowerSearchEngine {
        std::shared_ptr<symbolic::SymVariables> vars;
        
        symbolic::SymParamsMgr mgrParams; //Parameters for SymStateSpaceManager configuration.
        symbolic::SymParamsSearch searchParams; //Parameters to search the original state space

        const bool bidir;

        virtual void initialize_follower_search_engine() override;
        
    public:
        SymbolicFollowerSearchEngine(const Options &opts);
        virtual FollowerSolution solve (const std::vector<int> & leader_state, PlanReuse * plan_reuse) override;
        virtual FollowerSolution solve_minimum_ftask () override;
    }; 

}








#endif
