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

    BDD SymbolicStackelbergManager::get_static_follower (const std::vector<int> & leader_state) const {
        return vars->getPartialStateBDD(leader_state, pattern_vars_static_follower);
    }
    SymbolicStackelbergManager::SymbolicStackelbergManager(StackelbergTask * task_,
                                                           const Options & opts
        )  : task(task_), vars(make_shared<SymVariables>(opts)), mgr_params(opts),
             cost_type(OperatorCostFunction::get_cost_function()),
             stackelberg_variable_order(opts.get<bool> ("stackelberg_variable_order")) {

        if (stackelberg_variable_order) {
            vector<vector<int>> var_order_partitions(3);

            for(size_t var = 0; var < g_variable_domain.size(); ++var) {
                int partition = 1;
                
                if (task->is_leader_only_var(var)){
                    partition = 0;
                } else if (task->is_follower_only_var(var)) {
                    partition = 2;
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

        pattern_vars_static_follower.resize(g_variable_domain.size());
        for(size_t var = 0; var < g_variable_domain.size(); ++ var) {
            pattern_vars_static_follower[var] = task->is_follower_static_var(var);
        }       

        cubeFollowerSubproblems = vars->getCubePre(task->get_leader_only_vars()) *
            vars->getCubePre(task->get_follower_only_vars());

        mutex_bdds = make_shared<MutexBDDs> (vars.get(), g_mutex_groups, mgr_params, pattern_vars_follower_search);

        
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
            if (mgr_params.mutex_type == MutexType::MUTEX_EDELETION) {
                mutex_bdds->edeletion(*(follower_transitions_by_id[op.get_op_id()]));
            }

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

        for (const auto & entry : follower_transitions_by_leader_precondition) {
            bool failed = false;
            for (const auto & precondition : entry.first) {
                if(leader_state[precondition.first] != precondition.second) {
                    failed = true;
                    break;
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
                        indTRs[cost].push_back(*(follower_transitions_by_id[op->get_op_id()]));
                    }
                }
            }
        }

        for (const auto & goal : g_goal) {
            if (!pattern_vars_follower_search[goal.first] && leader_state[goal.first] != goal.second) {
                // Task is unsolvable. 
                return make_shared<StackelbergSS>(vars.get(), mgr_params, vars->zeroBDD(), vars->zeroBDD(),
                                                  indTRs, transitions, *mutex_bdds, pattern_vars_follower_search);
            }
        }

        BDD initialState = vars->getPartialStateBDD(leader_state, pattern_vars_follower_search);
        BDD goal = vars->getPartialStateBDD(g_goal);       

        assert(initialState == static_follower_initial_state*initialState);
        
        for (auto & trs : transitions) {
            merge(vars.get(), trs.second, mergeTR, mgr_params.max_tr_time, mgr_params.max_tr_size);
        }

        return make_shared<StackelbergSS>(vars.get(), mgr_params, initialState, goal, indTRs, transitions,
                                          *mutex_bdds, pattern_vars_follower_search);
    }

    
    // Obtains the follower manager where all actions that have been disabled
    std::shared_ptr<StackelbergSS> SymbolicStackelbergManager::get_follower_manager_minimal() {

              
        std::map<int, std::vector <symbolic::TransitionRelation>> indTRs;
        std::map<int, std::vector <symbolic::TransitionRelation>> transitions;


        //Here is where we should run the h2 preprocessor

        for (const auto & entry : follower_transitions_by_leader_precondition) {
            if (!entry.first.empty()) {
                continue;
            }
            
            for (const auto & tr_by_cost : entry.second) {
                int cost = tr_by_cost.first;

                for (const auto & tr : tr_by_cost.second) {
                    transitions[cost].push_back(tr);
                    for (const auto * op : tr.getOps()) {
                        indTRs[cost].push_back(*(follower_transitions_by_id[op->get_op_id()]));
                    }
                }
            }
        }

        for (const auto & goal : g_goal) {
            if (!pattern_vars_follower_search[goal.first] && g_initial_state_data[goal.first] != goal.second) {
                // Task is unsolvable. 
                return make_shared<StackelbergSS>(vars.get(), mgr_params, vars->zeroBDD(), vars->zeroBDD(),
                                                  indTRs, transitions, *mutex_bdds, pattern_vars_follower_search);
            }
        }

        BDD initialState = vars->getPartialStateBDD(g_initial_state_data, pattern_vars_follower_search);
        BDD goal = vars->getPartialStateBDD(g_goal);       
        
        for (auto & trs : transitions) {
            merge(vars.get(), trs.second, mergeTR, mgr_params.max_tr_time, mgr_params.max_tr_size);
        }

        return make_shared<StackelbergSS>(vars.get(), mgr_params, initialState, goal, indTRs, transitions,
                                          *mutex_bdds, pattern_vars_follower_search);
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

        return make_shared<StackelbergSS>(vars.get(), mgr_params, initialState, goal, indTRs, transitions,
                                          *mutex_bdds, vector<bool>());
    
    }


    std::shared_ptr<StackelbergSS> SymbolicStackelbergManager::get_empty_manager() const {

        std::map<int, std::vector <symbolic::TransitionRelation>>        indTRs;
        // for (const auto & entry : follower_transitions_by_leader_precondition) {
        //     if (!entry.first.empty()) {
        //         continue;
        //     }

        //     for (const auto & tr_by_cost : entry.second) {
        //         int cost = tr_by_cost.first;

        //         for (const auto & tr : tr_by_cost.second) {
        //             for (const auto * op : tr.getOps()) {
        //                 indTRs[cost].push_back(*(follower_transitions_by_id[op->get_op_id()]));
        //             }
        //         }
        //     }
        // }
        

        for (int op_no : task->get_global_operator_id_follower_ops()) {
            const GlobalOperator * op = &(g_operators[op_no]);
            auto * tr = transitions_by_id[op->get_op_id()].get();
            indTRs[tr->getCost()].push_back(*tr);
        }

        BDD initialState = vars->getStateBDD(g_initial_state());
        BDD goal = vars->zeroBDD();
        
        return make_shared<StackelbergSS>(vars.get(), mgr_params, initialState, goal,
                                          indTRs,
                                          std::map<int, std::vector <symbolic::TransitionRelation>> (),
                                          *mutex_bdds, vector<bool>());
    
    }

    int SymbolicStackelbergManager::get_cost (const GlobalOperator *op) const{
        return cost_type->get_adjusted_cost(op);
    }

    StackelbergSS::StackelbergSS(symbolic::SymVariables *vars,
                                 const symbolic::SymParamsMgr &params,
                                 BDD initialState, BDD goal,
                                 std::map<int, std::vector <symbolic::TransitionRelation>> indTRs_,
                                 std::map<int, std::vector <symbolic::TransitionRelation>> transitions,
                                 const MutexBDDs & mutex_bdds, const std::vector<bool> &  _pattern) :
        SymStateSpaceManager(vars, params, initialState, goal, transitions,
                             mutex_bdds.getValidStates(true),
                             mutex_bdds.getValidStates(false)),
                             pattern(_pattern), indTRs(indTRs_) {
        // cout << "StackelbergSS pattern: ";
        // for (auto v : pattern) {
        //     cout << v << ", ";
        // }
        // cout << endl;
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




    MutexBDDs::MutexBDDs (SymVariables * vars,
                          const std::vector<MutexGroup> &mutex_groups,
                          const symbolic::SymParamsMgr & p,
                          const::vector<bool> & relevant_vars) {
        //If (a) is initialized OR not using mutex OR edeletion does not need mutex
        if (p.mutex_type == MutexType::MUTEX_NOT) {
            return;     //Skip mutex initialization
        }

        bool genMutexBDD = true;
        bool genMutexBDDByFluent = (p.mutex_type == MutexType::MUTEX_EDELETION);

        if(genMutexBDDByFluent){
            //Initialize structure for exactlyOneBDDsByFluent (common to both init_mutex calls) 
            exactlyOneBDDsByFluent.resize(g_variable_domain.size());
            for (size_t i = 0; i < g_variable_domain.size(); ++i){
                exactlyOneBDDsByFluent[i].resize(g_variable_domain[i]); 
                for(int j = 0; j < g_variable_domain[i]; ++j){
                    exactlyOneBDDsByFluent[i][j] = vars->oneBDD();
                }
            }
        }
        init_mutex(vars, mutex_groups, p, genMutexBDD, genMutexBDDByFluent, false, relevant_vars);
        init_mutex(vars, mutex_groups, p, genMutexBDD, genMutexBDDByFluent, true, relevant_vars);
    }


    void MutexBDDs::init_mutex(SymVariables * vars,
                               const std::vector<MutexGroup> &mutex_groups,
                               const symbolic::SymParamsMgr &p, 
                               bool genMutexBDD, bool genMutexBDDByFluent, bool fw,
                               const::vector<bool> & relevant_vars) {
        DEBUG_MSG(cout << "Init mutex BDDs " << (fw ? "fw" : "bw") << ": "
                  << genMutexBDD << " " << genMutexBDDByFluent << endl;);

        vector<vector<BDD>> &notMutexBDDsByFluent = (fw ? notMutexBDDsByFluentFw : notMutexBDDsByFluentBw);

        vector<BDD> &notMutexBDDs = (fw ? notMutexBDDsFw : notMutexBDDsBw);

        int num_mutex = 0;
        int num_invariants = 0;

        if (genMutexBDDByFluent) {
            //Initialize structure for notMutexBDDsByFluent
            notMutexBDDsByFluent.resize(g_variable_domain.size());
            for (size_t i = 0; i < g_variable_domain.size(); ++i) {
                notMutexBDDsByFluent[i].resize(g_variable_domain[i]);
                for (int j = 0; j < g_variable_domain[i]; ++j) {
                    notMutexBDDsByFluent[i][j] = vars->oneBDD();
                }
            }
        }

        //Initialize mBDDByVar and invariant_bdds_by_fluent
        vector<BDD>  mBDDByVar;
        mBDDByVar.reserve(g_variable_domain.size());
        vector<vector<BDD>> invariant_bdds_by_fluent(g_variable_domain.size());
        for (size_t i = 0; i < invariant_bdds_by_fluent.size(); i++) {
            mBDDByVar.push_back(vars->oneBDD());
            invariant_bdds_by_fluent[i].resize(g_variable_domain[i]);
            for (size_t j = 0; j < invariant_bdds_by_fluent[i].size(); j++) {
                invariant_bdds_by_fluent[i][j] = vars->oneBDD();
            }
        }

        for (auto &mg : mutex_groups) {
            if (mg.pruneFW() != fw)
                continue;
            const vector<FactPair> &invariant_group = mg.getFacts();
            DEBUG_MSG(cout << mg << endl;);
            if (mg.isExactlyOne()) {
                BDD bddInvariant = vars->zeroBDD();
                int var = numeric_limits<int>::max();
                int val = 0;
                bool exactlyOneRelevant = true;
                for (auto &fluent : invariant_group) {
                    if (!relevant_vars[fluent.var]) {
                        exactlyOneRelevant = true;
                        break;
                    }
                    bddInvariant += vars->preBDD(fluent.var, fluent.value);
                    if (fluent.var < var) {
                        var = fluent.var;
                        val = fluent.value;
                    }
                }

                if (exactlyOneRelevant) {
                    num_invariants++;
                    if (genMutexBDD) {
                        invariant_bdds_by_fluent[var][val] *= bddInvariant;
                    }
                    if (genMutexBDDByFluent) {
                        for (auto &fluent : invariant_group) {
                            exactlyOneBDDsByFluent[fluent.var][fluent.value] *= bddInvariant;
                        }
                    }
                }
            }


            for (size_t i = 0; i < invariant_group.size(); ++i) {
                int var1 = invariant_group[i].var;
                if (!relevant_vars[var1])
                    continue;
                int val1 = invariant_group[i].value;
                BDD f1 = vars->preBDD(var1, val1);

                for (size_t j = i + 1; j < invariant_group.size(); ++j) {
                    int var2 = invariant_group[j].var;
                    if (!relevant_vars[var2])
                        continue;
                    int val2 = invariant_group[j].value;
                    BDD f2 = vars->preBDD(var2, val2);
                    BDD mBDD = !(f1 * f2);
                    if (genMutexBDD) {
                        num_mutex++;
                        mBDDByVar[min(var1, var2)] *= mBDD;
                        if (mBDDByVar[min(var1, var2)].nodeCount() > p.max_mutex_size) {
                            notMutexBDDs.push_back(mBDDByVar[min(var1, var2)]);
                            mBDDByVar[min(var1, var2)] = vars->oneBDD();
                        }
                    }
                    if (genMutexBDDByFluent) {
                        notMutexBDDsByFluent[var1][val1] *= mBDD;
                        notMutexBDDsByFluent[var2][val2] *= mBDD;
                    }
                }
            }
        }

        if (genMutexBDD) {
            for (size_t var = 0; var < g_variable_domain.size(); ++var) {
                if (!mBDDByVar[var].IsOne()) {
                    notMutexBDDs.push_back(mBDDByVar[var]);
                }
                for (const BDD &bdd_inv : invariant_bdds_by_fluent[var]) {
                    if (!bdd_inv.IsOne()) {
                        notMutexBDDs.push_back(bdd_inv);
                    }
                }
            }

            merge(vars, notMutexBDDs, mergeAndBDD,
                  p.max_mutex_time, p.max_mutex_size);
            std::reverse(notMutexBDDs.begin(), notMutexBDDs.end());
            // DEBUG_MSG(
            cout << "Mutex initialized " << (fw ? "fw" : "bw") << ". Total mutex added: " << num_mutex << " Invariant groups: " << num_invariants << endl;

        }
    }


    void MutexBDDs::edeletion(TransitionRelation & tr) const {
        tr.edeletion(notMutexBDDsByFluentFw, notMutexBDDsByFluentBw, exactlyOneBDDsByFluent);
    }


}






