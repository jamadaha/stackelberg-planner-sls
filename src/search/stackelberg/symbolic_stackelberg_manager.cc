#include "symbolic_stackelberg_manager.h"

#include "stackelberg_task.h"



#include "../utils/debug_macros.h"
#include "../mutex_group.h"
#include "../symbolic/sym_util.h"

#include <limits>
#include <algorithm>

using namespace std;
using namespace symbolic;

namespace stackelberg {

    SymbolicStackelbergManager::SymbolicStackelbergManager(StackelbergTask * task_,
                                                           const Options & opts
        )  : task(task_), vars(make_shared<SymVariables>(opts)), mgr_params(opts),
             cost_type(OperatorCostFunction::get_cost_function()) {

        vars->init();

        const auto & leader_only_vars = task->get_leader_only_vars();

        num_follower_bdd_vars = vars->getNumBDDVars();
        for (int var : leader_only_vars) {
            num_follower_bdd_vars -= ceil(log2(g_variable_domain[var]));
        }
        leaderOnlyVarsBDD = vars->getCubePre(task->get_leader_only_vars());
    }
    

    std::shared_ptr<StackelbergSS>
    SymbolicStackelbergManager::get_follower_manager(const std::vector<int> & leader_state) {
  
        BDD initialState = vars->getStateBDD(leader_state);
        BDD goal = vars->getPartialStateBDD(g_goal);

        vector<BDD> validStates;
        validStates.push_back(vars->oneBDD());

        std::map<int, std::vector <symbolic::TransitionRelation>> indTRs;
        std::map<int, std::vector <symbolic::TransitionRelation>> trs;

        for (int op_no : task->get_global_operator_id_follower_ops()) {
            const GlobalOperator *op = &(g_operators[op_no]);
            int cost = cost_type->get_adjusted_cost(op);
            indTRs[cost].push_back(TransitionRelation(vars.get(), op, cost));
            trs[cost].push_back(TransitionRelation(vars.get(), op, cost));
        }

        return make_shared<StackelbergSS>(vars.get(), mgr_params, initialState, goal, indTRs, trs, validStates);


        
    }
    // Obtains the follower manager where all actions that have been disabled
    std::shared_ptr<StackelbergSS> SymbolicStackelbergManager::get_follower_manager_minimal() {

        BDD initialState = vars->zeroBDD();//getStateBDD(leader_state);
        BDD goal = vars->getPartialStateBDD(g_goal);
        
        vector<BDD> validStates;
        validStates.push_back(vars->oneBDD());

        std::map<int, std::vector <symbolic::TransitionRelation>> indTRs;
        std::map<int, std::vector <symbolic::TransitionRelation>> trs;
        
        // for (size_t i = 0; i < g_operators.size(); i++) {
        //     const GlobalOperator *op = &(g_operators[i]);
        //     int cost = cost_type->get_adjusted_cost(i);
        //     DEBUG_MSG(cout << "Creating TR of op " << i << " of cost " << cost << endl;);
        //     indTRs[cost].push_back(TransitionRelation(vars, op, cost));
        //     if (p.mutex_type == MutexType::MUTEX_EDELETION) {
        //         indTRs[cost].back().edeletion(notMutexBDDsByFluentFw, notMutexBDDsByFluentBw, exactlyOneBDDsByFluent);
        //     }
        // }

        return make_shared<StackelbergSS>(vars.get(), mgr_params, initialState, goal, indTRs, trs, validStates);


    }

    BDD SymbolicStackelbergManager::get_follower_projection(BDD leader_search_states) const {
        return leader_search_states.AndAbstract(vars->oneBDD(), leaderOnlyVarsBDD);
    }

    std::shared_ptr<StackelbergSS> SymbolicStackelbergManager::get_leader_manager() {
        
        BDD initialState = vars->getStateBDD(g_initial_state());
        BDD goal = vars->zeroBDD();
        
        vector<BDD> validStates;
        validStates.push_back(vars->oneBDD());

        std::map<int, std::vector <symbolic::TransitionRelation>> indTRs;


        for (int op_no : task->get_global_operator_id_leader_ops()) {
            const GlobalOperator *op = &(g_operators[op_no]);
            int cost = cost_type->get_adjusted_cost(op);
            indTRs[cost].push_back(TransitionRelation(vars.get(), op, cost));
        }

        std::map<int, std::vector <symbolic::TransitionRelation>> transitions = indTRs;

        
        for (map<int, vector<TransitionRelation>>::iterator it = transitions.begin();
             it != transitions.end(); ++it) {
            merge(vars.get(), it->second, mergeTR, mgr_params.max_tr_time, mgr_params.max_tr_size);
        }

    return make_shared<StackelbergSS>(vars.get(), mgr_params, initialState, goal, indTRs, transitions, validStates);
    
    }


    StackelbergSS::StackelbergSS(symbolic::SymVariables *vars,
                                 const symbolic::SymParamsMgr &params,
                                 BDD initialState, BDD goal,
                                 std::map<int, std::vector <symbolic::TransitionRelation>> indTRs_,
                                 std::map<int, std::vector <symbolic::TransitionRelation>> transitions,
                                 std::vector<BDD> validStates) :
        SymStateSpaceManager(vars, params, initialState, goal, transitions, validStates), indTRs(indTRs_) {
        
    }


    std::string StackelbergSS::tag() const {
        return "stackelberg";
    }

    BDD StackelbergSS::shrinkExists(const BDD &bdd, int ) const {
        return bdd;
    }
    
    BDD StackelbergSS::shrinkForall(const BDD &bdd, int ) const {
        return bdd;
    }
    
    BDD StackelbergSS::shrinkTBDD(const BDD &bdd, int ) const {
        return bdd;
    }


}






