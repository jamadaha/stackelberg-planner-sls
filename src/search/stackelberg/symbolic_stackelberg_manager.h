#ifndef SYMBOLIC_STACKELBERG_MANAGER_H
#define SYMBOLIC_STACKELBERG_MANAGER_H

#include "../symbolic/sym_state_space_manager.h"

class OptionParser;
class Options;

class MutexGroup;

namespace stackelberg {

    class StackelbergTask;

    class MutexBDDs {

        std::vector<BDD> notMutexBDDsFw, notMutexBDDsBw;
        //notMutex relative for each fluent
        std::vector<std::vector<BDD>> notMutexBDDsByFluentFw, notMutexBDDsByFluentBw;
        std::vector<std::vector<BDD>> exactlyOneBDDsByFluent;


    public:
        
        MutexBDDs (symbolic::SymVariables * vars,
                   const std::vector<MutexGroup> &mutex_groups,
                   const symbolic::SymParamsMgr &params,
                   const std::vector<bool> & relevant_vars);
        
        void init_mutex(symbolic::SymVariables * vars,
                        const std::vector<MutexGroup> &mutex_groups,
                        const symbolic::SymParamsMgr &params, 
                        bool genMutexBDD, bool genMutexBDDByFluent, bool fw,
                        const std::vector<bool> & relevant_vars);

        const std::vector<BDD> & getValidStates(bool fw) const{
            if (fw) return notMutexBDDsFw;
            else return notMutexBDDsBw;
        }

        void edeletion(symbolic::TransitionRelation & tr) const;

    }; 
    class StackelbergSS : public symbolic::SymStateSpaceManager {

        std::vector<bool> pattern;
        
        //Individual TRs: Useful for shrink and plan construction
        std::map<int, std::vector <symbolic::TransitionRelation>> indTRs;

    public:
        StackelbergSS(symbolic::SymVariables *v, const symbolic::SymParamsMgr &params,
                      BDD initialState, BDD goal,
                      std::map<int, std::vector <symbolic::TransitionRelation>> indTRs_,
                      std::map<int, std::vector <symbolic::TransitionRelation>> trs,
                      const MutexBDDs & mutex_bdds, const std::vector<bool> &  _pattern);

        virtual ~StackelbergSS() = default;


        //For plan solution reconstruction. Only avaialble in original state space
        virtual const std::map<int, std::vector <symbolic::TransitionRelation>> &getIndividualTRs() const override {
            return indTRs;
        }

        virtual std::string tag() const override;

        virtual BDD shrinkExists(const BDD &bdd, int maxNodes) const override;
        virtual BDD shrinkForall(const BDD &bdd, int maxNodes) const override;
        virtual BDD shrinkTBDD(const BDD &bdd, int maxNodes) const override;

        const std::vector<bool> & get_relevant_vars() const {
            return pattern;
        }                
    };


    typedef  std::vector<std::pair<int,int>> LeaderPrecondition;
    // This class initializes common data structures that are useful for all stackelberg
    // searches.
    class SymbolicStackelbergManager {
        StackelbergTask* task;
        
        std::shared_ptr<symbolic::SymVariables> vars;

        symbolic::SymParamsMgr mgr_params;

        std::shared_ptr<OperatorCostFunction> cost_type;
            
        const bool stackelberg_variable_order;
        
        BDD cubeFollowerSubproblems;
        int num_bdd_vars_follower_subproblems;
        std::vector<bool> pattern_vars_follower_subproblems;
        std::vector<bool> pattern_vars_follower_search;
        std::vector<bool> pattern_vars_static_follower;


        std::shared_ptr<MutexBDDs> mutex_bdds;
        
        BDD cubeOnlyFollowerVars;
        
        BDD static_follower_initial_state;
        
        //List of transition relations by ID. Needed for plan reconstruction
        mutable std::unique_ptr<std::map<int, std::vector<symbolic::TransitionRelation>>> transition_relation;

        std::vector<std::unique_ptr<symbolic::TransitionRelation>> transitions_by_id;
        std::vector<std::unique_ptr<symbolic::TransitionRelation>> follower_transitions_by_id;
        std::map <LeaderPrecondition, std::map<int, std::vector<symbolic::TransitionRelation>>> follower_transitions_by_leader_precondition;
        
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



        const symbolic::TransitionRelation &  get_follower_transition_relation(const GlobalOperator * op) const;
        const symbolic::TransitionRelation &  get_transition_relation(const GlobalOperator * op) const;
        const std::map<int, std::vector<symbolic::TransitionRelation> > &  get_transition_relation() const;

        std::vector<int> sample_follower_initial_state(BDD follower_initial_states) const;

        BDD get_follower_initial_state_projection(BDD leader_search_states) const;

        BDD get_follower_initial_state (const std::vector<int> & leader_state) const;
        
        
        int get_num_follower_bdd_vars() const {
            return num_bdd_vars_follower_subproblems;
        }

        std::shared_ptr<symbolic::SymVariables> get_sym_vars() {
            return vars;
        }

        const std::vector<bool> & get_pattern_vars_follower_subproblems() const {
            return pattern_vars_follower_subproblems;
        }

        BDD get_static_follower (const std::vector<int> & leader_state) const; 

        const BDD & get_static_follower_initial_state () const {
            return static_follower_initial_state;
        }

        std::shared_ptr<StackelbergSS> get_empty_manager() const;
            

        int get_cost (const GlobalOperator *op) const; 

        static void add_options_to_parser(OptionParser &parser);
    };

    
    }
#endif

