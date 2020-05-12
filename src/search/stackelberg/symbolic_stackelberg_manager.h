#ifndef SYMBOLIC_STACKELBERG_MANAGER_H
#define SYMBOLIC_STACKELBERG_MANAGER_H

#include "../symbolic/sym_state_space_manager.h"

class MutexGroup; 
namespace stackelberg {

    class StackelbergTask;
    
    class StackelbergSS : public symbolic::SymStateSpaceManager {       
        //Individual TRs: Useful for shrink and plan construction
        std::map<int, std::vector <symbolic::TransitionRelation>> indTRs;

    public:

        StackelbergSS(symbolic::SymVariables *v, const symbolic::SymParamsMgr &params,
                      BDD initialState, BDD goal,
                      std::map<int, std::vector <symbolic::TransitionRelation>> indTRs_,
                      std::map<int, std::vector <symbolic::TransitionRelation>> trs,
                      std::vector<BDD> validStates);


        //For plan solution reconstruction. Only avaialble in original state space
        virtual const std::map<int, std::vector <symbolic::TransitionRelation>> &getIndividualTRs() const override {
            return indTRs;
        }

        virtual std::string tag() const override;

        virtual BDD shrinkExists(const BDD &bdd, int maxNodes) const override;
        virtual BDD shrinkForall(const BDD &bdd, int maxNodes) const override;
        virtual BDD shrinkTBDD(const BDD &bdd, int maxNodes) const override;
    };


    
    typedef  std::vector<std::pair<int,int>> LeaderPrecondition;
    // This class initializes common data structures that are useful for all stackelberg
    // searches.
    class SymbolicStackelbergManager {
        StackelbergTask* task;
        
        std::shared_ptr<symbolic::SymVariables> vars;

        symbolic::SymParamsMgr mgr_params;

        std::shared_ptr<OperatorCostFunction> cost_type;

        BDD leaderOnlyVarsBDD;
        int num_follower_bdd_vars;

        // Transitions and mutex BDDs are classified by leader precondition. An important
        // question is, should TRs have encoded the leader precondition already? There are
        // three options: fully encoded, partially encoded, no encoded.

        // Fully encoded: All follower searches use exactly the same TRs. Leader variables
        // fully form part of everything.
        //    * Advantage: It's simpler because all operations occur under the same representation.
        //                 The only thing required to perform a follower search is to
        //
        //    * Disadvantage: Backward search becomes stupid, trying to apply actions that
        //    * depend on preconditions of leader actions.
        //
        //

        // If the leader preconditions are encoded, there will be a slightly innecessary
        // overhead on the symbolic searches.x
      
        /* std::map<int, std::vector <std::pair<int, symbolic::TransitionRelation>>> follower_transitions_by_leader_precondition;  */
        /* std::map<int, std::vector <symbolic::TransitionRelation>> transitions_by_leader_precondition; //TRs by cost */
        /* std::vector<std::pair<LeaderPrecondition, BDD>> validStatesFw, validStatesBw; */

               
    public:
        SymbolicStackelbergManager(StackelbergTask* task, const Options & opts);
        

        // Obtains the follower manager for leader state
        // Returns the set of TRs that are compatible with the leader state
        // Sets up the initial state based on the leader state
        std::shared_ptr<StackelbergSS> get_follower_manager(const std::vector<int> & leader_state);
        
        // Obtains the follower manager where all actions that have been disabled
        std::shared_ptr<StackelbergSS> get_follower_manager_minimal();

        std::shared_ptr<StackelbergSS> get_leader_manager();


        std::map<int, BDD> regress_plan(const std::vector<const GlobalOperator *> & plan);


        BDD get_follower_projection(BDD leader_search_states) const;
        int get_num_follower_bdd_vars() const {
            return num_follower_bdd_vars;
        }

        std::shared_ptr<symbolic::SymVariables> get_sym_vars() {
            return vars;
        }                        
    };

    
}
#endif

