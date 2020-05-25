#ifndef STACKELBERG_PLAN_REUSE_H
#define STACKELBERG_PLAN_REUSE_H

#include <memory>

#include "../symbolic/sym_variables.h"
#include "../symbolic/unidirectional_search.h"
#include "../symbolic/sym_solution.h"
#include "../symbolic/closed_list.h"

/* We keep a plan_registry, where each action sequence is mapped to a unique_id, which is
 * what is actually stored in each node. That way, we can easily keep for every node what
 * is the parent plan. */


class Options;
class OptionParser;


namespace stackelberg {

    class SymbolicStackelbergManager;
    class StackelbergSS;

    
    class OppositeFrontierExplicit {
        std::shared_ptr<symbolic::ClosedList> closed_list;
        int desired_bound;

    public:
    OppositeFrontierExplicit(std::shared_ptr<symbolic::ClosedList> closed) :
        closed_list (closed), desired_bound(0) {}
        
        int check_goal_cost(const GlobalState & state){
            return closed_list->check_goal_cost(state);
        }
    };

    
    class PlanReuse {
    protected:

        const bool check_all_cuts;

        std::shared_ptr<StackelbergSS> mockup_mgr;
        std::shared_ptr<SymbolicStackelbergManager> stackelberg_mgr;
        std::shared_ptr<symbolic::SymVariables> vars;

        std::shared_ptr<symbolic::ClosedList> closed_list;
        int current_follower_bound;
        
    public:

    PlanReuse() : check_all_cuts(false) {
        }
        
        void initialize(std::shared_ptr<SymbolicStackelbergManager> mgr);

        virtual void initialize() = 0;

        void set_follower_bound(int bound) {
            current_follower_bound = bound;     
        }

        int get_follower_bound() const {
            return current_follower_bound;
        }

        std::shared_ptr<symbolic::ClosedList> get_closed() {
            return closed_list;
        }

        // Regress the plan and filter the BDD of follower initial states to eliminate the
        // ones for which the plan applies
        virtual BDD regress_plan_to_follower_initial_states(const std::vector<const GlobalOperator *> & plan,
                                                            const BDD & follower_initial_states) = 0;
        
        // Filter the BDD of follower initial states to eliminate the ones for which a
        // solution is known
        virtual BDD find_plan_follower_initial_states (const BDD & bdd) const = 0; 


        std::shared_ptr<symbolic::OppositeFrontier> get_opposite_frontier(const std::vector<int> & leader_state) const;
        
        std::shared_ptr<OppositeFrontierExplicit> get_opposite_frontier_explicit(const std::vector<int> & leader_state) const;


        void load_plans (const symbolic::ClosedList & closed) const; 

        
        /* symbolic::SymSolution checkCut(const symbolic::PlanReconstruction * search, const BDD &states, int g, bool fw) const override; */

	/* virtual BDD notClosed () const override; */

	/* //Returns true only if all not closed states are guaranteed to be dead ends */
	/* virtual bool exhausted () const override { */
        /*     return false; */
        /* } */
	
	/* virtual int getHNotClosed() const override { */
        /*     return 0; */
        /* } */

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
        virtual  BDD regress_plan_to_follower_initial_states(const std::vector<const GlobalOperator *> & plan,
                                                             const BDD & follower_initial_states) override; 
        virtual BDD find_plan_follower_initial_states (const BDD & bdd) const override; 
        /* virtual bool find_plan (const GlobalState & state, int desired_bound) const override;       */
    };

    class PlanReuseRegressionSearch : public PlanReuse {

    public:

        PlanReuseRegressionSearch (const Options & opts);

        virtual void initialize() override;

        virtual  BDD regress_plan_to_follower_initial_states(const std::vector<const GlobalOperator *> & plan,
                                                             const BDD & follower_initial_states) override; 
        virtual BDD find_plan_follower_initial_states (const BDD & bdd) const override; 
        /* virtual bool find_plan (const GlobalState & state, int desired_bound) const override;       */

    };

    

}

#endif
