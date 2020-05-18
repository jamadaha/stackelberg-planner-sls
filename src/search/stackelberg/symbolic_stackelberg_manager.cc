#include "symbolic_stackelberg_manager.h"

#include "stackelberg_task.h"


#include "../option_parser.h"

#include "../utils/debug_macros.h"
#include "../mutex_group.h"
#include "../symbolic/sym_util.h"

#include "../symbolic/opt_order.h"

#include <limits>
#include <algorithm>

using namespace std;
using namespace symbolic;

namespace stackelberg {

    void SymbolicStackelbergManager::add_options_to_parser(OptionParser &parser) {
	SymVariables::add_options_to_parser(parser);
	SymParamsMgr::add_options_to_parser(parser);
        
        parser.add_option<bool> ("stackelberg_variable_order",
                                 "Ensure that variables are ordered according to the stackelberg task", "true");
    }


    SymbolicStackelbergManager::SymbolicStackelbergManager(StackelbergTask * task_,
                                                           const Options & opts
        )  : task(task_), vars(make_shared<SymVariables>(opts)), mgr_params(opts),
             cost_type(OperatorCostFunction::get_cost_function()),
             stackelberg_variable_order(opts.get<bool> ("stackelberg_variable_order")) {

        if (stackelberg_variable_order) {
            vector<vector<int>> var_order_partitions(3);

            for(size_t var = 0; var < g_variable_domain.size(); ++var) {
                int partition = 2;
                
                if (task->is_leader_only_var(var)){
                    partition = 0;
                } else if (task->is_follower_only_var(var)) {
                    partition = 1;
                }
                    
                var_order_partitions[partition].push_back(var);
            }
            
            auto var_order = InfluenceGraph::compute_gamer_ordering(var_order_partitions);

            vars->init(var_order);
        } else {
            vars->init();
        }


        pattern_vars_follower_subproblems.resize(g_variable_domain.size(), true);
        num_bdd_vars_follower_subproblems = vars->getNumBDDVars();
        for (int var : task->get_leader_only_vars()) {
            num_bdd_vars_follower_subproblems -= ceil(log2(g_variable_domain[var]));
            pattern_vars_follower_subproblems[var] = false;
        }
        for (int var : task->get_follower_only_vars()) {
            num_bdd_vars_follower_subproblems -= ceil(log2(g_variable_domain[var]));
            pattern_vars_follower_subproblems[var] = false;
        }


        pattern_vars_follower_search = task->get_follower_vars ();

        cubeFollowerSubproblems = vars->getCubePre(task->get_leader_only_vars()) *
            vars->getCubePre(task->get_follower_only_vars());
        

        transitions_by_id.resize(g_operators.size());
        follower_transitions_by_id.resize(g_operators.size());

        for (int op_no : task->get_global_operator_id_follower_ops()) {
            const GlobalOperator & op = g_operators[op_no];

            LeaderPrecondition leader_precondition;
            for (const auto & prevail : op.get_preconditions()) { //Put precondition of label
                if(!task->is_follower_effect_var(prevail.var)) {
                    leader_precondition.push_back(make_pair(prevail.var, prevail.val));
                }
            }
                                                       
            int cost = cost_type->get_adjusted_cost(&op);

            transitions_by_id[op.get_op_id()] = make_unique<TransitionRelation>(vars.get(), &op, cost);
            follower_transitions_by_id[op.get_op_id()] = make_unique<TransitionRelation>(vars.get(), &op, cost, pattern_vars_follower_search);
            follower_transitions_by_leader_precondition[leader_precondition][cost].push_back(*(follower_transitions_by_id[op.get_op_id()]));
        }

        for (auto & entry : follower_transitions_by_leader_precondition) {
            for (auto & trs : entry.second) {
                merge(vars.get(), trs.second, mergeTR, mgr_params.max_tr_time, mgr_params.max_tr_size);
            }
        }
        
        std::vector<bool> pattern_vars_static_initial_state (g_variable_domain.size(), false);
        for (int var : task->get_follower_only_vars()) {
            pattern_vars_static_initial_state[var] = true;
        }

        static_follower_initial_state = vars->getPartialStateBDD(g_initial_state_data,
                                                                 pattern_vars_static_initial_state);
    }
    
    std::shared_ptr<StackelbergSS>
    SymbolicStackelbergManager::get_follower_manager(const std::vector<int> & leader_state) {

        //Variables that the follower can modify given the leader state
        // std::vector<bool> follower_vars (g_variable_domain.size(), true); 
      
        std::map<int, std::vector <symbolic::TransitionRelation>> indTRs;
        std::map<int, std::vector <symbolic::TransitionRelation>> transitions;


        //Here is where we should run the h2 preprocessor

        vector<BDD> validStates;
        validStates.push_back(vars->oneBDD());

        for (const auto & entry : follower_transitions_by_leader_precondition) {
            bool failed = false;
            for (const auto & precondition : entry.first) {
                // follower_vars[precondition.first]=false;
                if(leader_state[precondition.first] != precondition.second) {
                    failed = true;
                    //break;
                }
            }
            if (failed) {
                continue;
            }
            
            for (const auto & tr_by_cost : entry.second) {
                int cost = tr_by_cost.first;

                for (const auto & tr : tr_by_cost.second) {
                    transitions[cost].push_back(tr);
                    for (const auto * op : tr.getOps()) {
                        // for (const auto & eff : op->get_effects()) {
                        //     follower_vars[eff.var] = true;
                        // }
                        indTRs[cost].push_back(*(follower_transitions_by_id[op->get_op_id()]));
                    }
                }
            }
        }

        for (const auto & goal : g_goal) {
            if (!pattern_vars_follower_search[goal.first] && leader_state[goal.first] != goal.second) {
                // Task is unsolvable. 
                return make_shared<StackelbergSS>(vars.get(), mgr_params, vars->zeroBDD(), vars->zeroBDD(), indTRs, transitions, validStates, pattern_vars_follower_search);
            }
        }

        BDD initialState = vars->getPartialStateBDD(leader_state, pattern_vars_follower_search);
        BDD goal = vars->getPartialStateBDD(g_goal);       

        assert(initialState == static_follower_initial_state*initialState);
        
        for (auto & trs : transitions) {
            merge(vars.get(), trs.second, mergeTR, mgr_params.max_tr_time, mgr_params.max_tr_size);
        }

        return make_shared<StackelbergSS>(vars.get(), mgr_params, initialState, goal, indTRs, transitions, validStates, pattern_vars_follower_search);
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

        return make_shared<StackelbergSS>(vars.get(), mgr_params, initialState, goal, indTRs, trs, validStates, vector<bool>());


    }

    BDD SymbolicStackelbergManager::get_follower_initial_state_projection(BDD leader_search_states) const {
        return leader_search_states.AndAbstract(vars->oneBDD(), cubeFollowerSubproblems);
    }

    
    std::vector<int>
    SymbolicStackelbergManager::sample_follower_initial_state(BDD follower_initial_states) const {
        auto sampled_state =  vars->sample_state(follower_initial_states, pattern_vars_follower_subproblems);
        
#ifndef NDEBUG
        cout << "Sampled state" << endl;
        for (size_t v = 0; v < g_variable_domain.size(); ++v) {           
            if(!task->is_leader_only_var(v))  {
                cout << g_fact_names[v][sampled_state[v]]  << endl;
            }
        }

         for (size_t v = 0; v < g_variable_domain.size(); ++v) {           
             assert(pattern_vars_follower_subproblems[v] || sampled_state[v] == g_initial_state_data[v]);
         }
#endif

        return sampled_state;
    }


    const TransitionRelation &  SymbolicStackelbergManager::get_transition_relation(const GlobalOperator * op) const {
        return *(transitions_by_id.at(op->get_op_id()));
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

        return make_shared<StackelbergSS>(vars.get(), mgr_params, initialState, goal, indTRs, transitions, validStates, vector<bool>());
    
    }


    std::shared_ptr<StackelbergSS> SymbolicStackelbergManager::get_empty_manager() const {
        
        BDD initialState = vars->getStateBDD(g_initial_state());
        BDD goal = vars->zeroBDD();
        
        vector<BDD> validStates;
        validStates.push_back(vars->oneBDD());

        return make_shared<StackelbergSS>(vars.get(), mgr_params, initialState, goal,
                                          std::map<int, std::vector <symbolic::TransitionRelation>> (),
                                          std::map<int, std::vector <symbolic::TransitionRelation>> (), validStates, vector<bool>());
    
    }

    int SymbolicStackelbergManager::get_cost (const GlobalOperator *op) const{
        return cost_type->get_adjusted_cost(op);
    }

    StackelbergSS::StackelbergSS(symbolic::SymVariables *vars,
                                 const symbolic::SymParamsMgr &params,
                                 BDD initialState, BDD goal,
                                 std::map<int, std::vector <symbolic::TransitionRelation>> indTRs_,
                                 std::map<int, std::vector <symbolic::TransitionRelation>> transitions,
                                 std::vector<BDD> validStates, const std::vector<bool> &  _pattern) :
        SymStateSpaceManager(vars, params, initialState, goal, transitions, validStates),
        pattern(_pattern), indTRs(indTRs_) {
        
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






