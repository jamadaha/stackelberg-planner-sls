#ifndef STACKELBERG_PLAN_REUSE_H
#define STACKELBERG_PLAN_REUSE_H

#include <memory>

#include "../symbolic/sym_variables.h"
#include "../symbolic/unidirectional_search.h"
#include "../symbolic/sym_solution.h"

#include "../symbolic/closed_list_disj.h"
#include "../symbolic/closed_list.h"

/* We keep a plan_registry, where each action sequence is mapped to a unique_id, which is
 * what is actually stored in each node. That way, we can easily keep for every node what
 * is the parent plan. */


class Options;
class OptionParser;

namespace stackelberg {

    class SymbolicStackelbergManager;
    class StackelbergSS;
    class FollowerSolution;
    
    class OppositeFrontierExplicit {
        //We keep two different frontiers: one for the lower and one for the upper bound.

        std::shared_ptr<symbolic::ClosedListDisj> closed_list_lower;        
        std::shared_ptr<symbolic::ClosedListDisj> closed_list_upper;
        int desired_bound;
        

    public:
    OppositeFrontierExplicit(std::shared_ptr<symbolic::ClosedListDisj> closed, int desired_bound_) :
        closed_list_upper (closed), desired_bound(desired_bound_) {}
        
        int check_goal_cost(const GlobalState & state){
            return closed_list_upper->check_goal_cost(state);
        }

        void getPlan(const GlobalState & state, int g, std::vector <const GlobalOperator *> &path) const;

        int get_desired_bound() const {
            return desired_bound;
        }
    };

    
    class PlanReuse {
    protected:
        const bool check_all_cuts;

        std::shared_ptr<StackelbergSS> mockup_mgr;
        std::shared_ptr<SymbolicStackelbergManager> stackelberg_mgr;
        std::shared_ptr<symbolic::SymVariables> vars;

        std::shared_ptr<symbolic::ClosedListDisj> closed_list_upper;
        int current_follower_bound;
        
    public:

    PlanReuse() : check_all_cuts(false) {
        }
        
    virtual ~PlanReuse() = default;
        void initialize(std::shared_ptr<SymbolicStackelbergManager> mgr);

        virtual void initialize() = 0;

        void set_follower_bound(int bound) {
            current_follower_bound = bound;     
        }

        int get_follower_bound() const {
            return current_follower_bound;
        }

        std::shared_ptr<symbolic::ClosedListDisj> get_closed() {
            return closed_list_upper;
        }

        // Regress the plan and filter the BDD of follower initial states to eliminate the
        // ones for which the plan applies
        virtual BDD regress_plan_to_follower_initial_states(const FollowerSolution & sol,
                                                            const BDD & follower_initial_states) = 0;
        
        // Filter the BDD of follower initial states to eliminate the ones for which a
        // solution is known
        virtual BDD find_plan_follower_initial_states (const BDD & bdd) const = 0; 


        std::shared_ptr<symbolic::OppositeFrontier> get_opposite_frontier(const std::vector<int> & leader_state) const;
        
        std::shared_ptr<OppositeFrontierExplicit> get_opposite_frontier_explicit(const std::vector<int> & leader_state) const;


        void load_plans (const symbolic::ClosedList & closed) const; 

    };


    
    class PlanReuseSimple : public PlanReuse {
                
        // As the bound always increases, we can simply store a single BDD with all states
        // with a value lower or equal than the bound
        BDD solvedFollowerInitialStates;

        std::map<int, BDD> boundsForFollowerStates;

        const bool accumulate_intermediate_states;

    public:
        PlanReuseSimple(const Options & opts);

        virtual void initialize() override;
        virtual  BDD regress_plan_to_follower_initial_states(const FollowerSolution & plan,
                                                             const BDD & follower_initial_states) override; 
        virtual BDD find_plan_follower_initial_states (const BDD & bdd) const override; 
        /* virtual bool find_plan (const GlobalState & state, int desired_bound) const override;       */

        BDD regress_multiple_plans_to_follower_initial_states (const FollowerSolution & sol,
                                                               const BDD & follower_initial_states);

    };

    class PlanReuseRegressionSearch : public PlanReuse {

        BDD solvedFollowerInitialStates;

        std::map<int, BDD> boundsForFollowerStates;

        const bool accumulate_intermediate_states;

        const int max_nodes_regression;

        BDD preimage(const BDD & origin, const symbolic::TransitionRelation & tr, int f, int g, int c,
                     BDD accum_result, int nodeLimit, const std::map<int, BDD> & heuristic) const;

    public:

        PlanReuseRegressionSearch (const Options & opts);

        virtual void initialize() override;

        virtual  BDD regress_plan_to_follower_initial_states(const FollowerSolution & plan,
                                                             const BDD & follower_initial_states) override; 
        virtual BDD find_plan_follower_initial_states (const BDD & bdd) const override; 
        /* virtual bool find_plan (const GlobalState & state, int desired_bound) const override;       */

    };

    

}

#endif
