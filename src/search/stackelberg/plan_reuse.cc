
#include "plan_reuse.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "../global_operator.h"

#include "../symbolic/sym_variables.h"
#include "../symbolic/closed_list_disj.h"
#include "../symbolic/open_list.h"
#include "follower_search_engine.h"

#include "symbolic_stackelberg_manager.h"

using namespace symbolic;
using namespace std;

namespace stackelberg {

    std::shared_ptr<symbolic::OppositeFrontier>
    PlanReuse::get_opposite_frontier(const std::vector<int> & leader_state) const {
        BDD bdd = stackelberg_mgr->get_static_follower(leader_state);
        return make_shared<ClosedListDisj> (*closed_list_upper, bdd);
    }

    std::shared_ptr<OppositeFrontierExplicit>
    PlanReuse::get_opposite_frontier_explicit(const std::vector<int> & leader_state) const {
        BDD bdd = stackelberg_mgr->get_static_follower(leader_state);
        return make_shared<OppositeFrontierExplicit> (make_shared<ClosedListDisj> (*closed_list_upper, bdd), current_follower_bound);
    }


    void OppositeFrontierExplicit::getPlan(const GlobalState & state, int g, std::vector <const GlobalOperator *> &path) const {
        BDD cut = closed_list_upper->getVars()->getStateBDD(state); 
        closed_list_upper->getPlan(cut, g, false, path );
    }



    void PlanReuse::initialize(std::shared_ptr<SymbolicStackelbergManager> mgr) {
        stackelberg_mgr = mgr;
        vars = mgr->get_sym_vars();

        current_follower_bound = 0;

        mockup_mgr = mgr->get_empty_manager();
        closed_list_upper = make_shared<ClosedListDisj>();
        closed_list_upper->init(mockup_mgr.get());
        initialize();
    }

    void PlanReuse::load_plans (const ClosedList & closed) const {
        closed_list_upper->load(closed);
    }

    PlanReuseSimple::PlanReuseSimple (const Options & opts) :
        accumulate_intermediate_states (opts.get<bool>("accumulate_intermediate_states")) {
    }
        
    void PlanReuseSimple::initialize() {
        solvedFollowerInitialStates = vars->zeroBDD();
    }

   
    BDD PlanReuseSimple::find_plan_follower_initial_states (const BDD & bdd) const {
        return bdd - solvedFollowerInitialStates;
    }

    // bool PlanReuseSimple::find_plan (const GlobalState & , int ) const {
    //     return false;
    // }


    
    BDD PlanReuseSimple::regress_plan_to_follower_initial_states
    (const FollowerSolution & sol,  const BDD & follower_initial_states) {

        const std::vector<const GlobalOperator *> & plan = sol.get_plan();
        BDD current = vars->getPartialStateBDD(g_goal);
        BDD result = vars->zeroBDD();
        int cost = 0;
        int zeroCostSteps = 0;
        
        closed_list_upper->insertWithZeroCostSteps(cost, zeroCostSteps, current);
        for (int i = plan.size() -1; i >= 0; --i ){
            if (accumulate_intermediate_states) {
                result += current;
            }

            //cout << "Regressing plan " <<  cost << " " << plan[i]->get_name() << endl;
            current = stackelberg_mgr->get_transition_relation(plan[i]).preimage(current);
            int step_cost = stackelberg_mgr->get_cost(plan[i]);
            cost += step_cost;

            if (step_cost == 0) {
                zeroCostSteps  ++;
            } else {
                zeroCostSteps = 0;
            }
            
            closed_list_upper->insertWithZeroCostSteps(cost, zeroCostSteps, current);
        }

        result += current;
        
        if (accumulate_intermediate_states) {
            result *= stackelberg_mgr->get_static_follower_initial_state ();
        }
        result = stackelberg_mgr->get_follower_initial_state_projection(result);

        solvedFollowerInitialStates += result;
        return follower_initial_states - result;
    }



    
    PlanReuseRegressionSearch::PlanReuseRegressionSearch (const Options & opts) :
        accumulate_intermediate_states (opts.get<bool>("accumulate_intermediate_states")),
        max_nodes_regression(opts.get<int>("max_nodes_regression")) {
    }
        
    void PlanReuseRegressionSearch::initialize() {
        solvedFollowerInitialStates = vars->zeroBDD();
    }

   
    BDD PlanReuseRegressionSearch::find_plan_follower_initial_states (const BDD & bdd) const {
        return bdd - solvedFollowerInitialStates;
    }



    // Auxiliary function to perform preimages with the limit
    BDD PlanReuseRegressionSearch::image(bool fw, const BDD & origin, const TransitionRelation & tr, int f, int g, int c,
                                            BDD accum_result, int nodeLimit, const map<int, BDD> & heuristic) const {

        BDD newStates;

        try {
            if (fw) {
                newStates = tr.image(origin, nodeLimit);
            } else {
                newStates = tr.preimage(origin, nodeLimit);
            }
        }catch(const BDDError &) {
            BDD originNew = origin;
            if (heuristic.count(f-g)) {
                originNew *= heuristic.at(f-g);
            }
            if (fw) {
                newStates = tr.image(originNew);
            } else {
                newStates = tr.preimage(originNew);
            }
        }

        try {
            if (accum_result.nodeCount() > nodeLimit) {
                if (!heuristic.count(f-g-c)) {
                    cout << "Should make zero because there is no " << f-g-c << endl;
                    // for (auto entry  : heuristic){
                    //     cout << entry.first << endl;
                    // }
                    // return vars->zeroBDD();
                    accum_result += newStates;

                } else {
                    cout << "Conjoin with heuristic: " << f-g-c << endl;
                    cout << newStates.nodeCount() << endl;
                    cout << heuristic.at(f-g-c).nodeCount() << endl;
                
                    accum_result *= heuristic.at(f-g-c);
                    accum_result += newStates*heuristic.at(f-g-c);
                }                
            }else {
                accum_result = accum_result.Or(newStates, nodeLimit);
            }
        }catch(const BDDError &) {
            if (!heuristic.count(f-g-c)) {
                cout << "Making zero because there is no " << f-g-c << endl;

                return vars->zeroBDD();
            }
            accum_result *= heuristic.at(f-g-c);
            accum_result += newStates*heuristic.at(f-g-c);
        }

        return accum_result;
    }


    void PlanReuseRegressionSearch::astar_perfect_search (bool fw, const std::map<int, std::vector<symbolic::TransitionRelation>> & trs, int solution_cost, const BDD & init, int g, const std::map<int, BDD> & solvedWith,  int max_nodes_limit,
                                                          std::function<void(const BDD &, int, int)> process_expanded) const{
       std::map<int, BDD> open;

        open[g] = init;
        
        while (!open.empty()) {
            g = open.begin()->first;
            BDD current = open.begin()->second;
            if (current.IsZero()) {
                open.erase(open.begin());
                continue;
            }

            cout << "Reconstructing " << g << "/" << solution_cost  << ": " << current.nodeCount() << endl;
            if (current.nodeCount() > max_nodes_limit) {
                current *= solvedWith.count(solution_cost-g) ? solvedWith.at(solution_cost-g) : vars->zeroBDD()  ;
            }


            process_expanded(current, g, 0);
            // if (result && (accumulate_intermediate_states || g == solution_cost)) {
            //     (*result) += stackelberg_mgr->get_follower_initial_state_projection(current*stackelberg_mgr->get_static_follower_initial_state ());
            // }
            // closed_list_upper->insertWithZeroCostSteps(g, zeroCostSteps, currentZero);
            
            if (trs.count(0)) {
                int zeroCostSteps = 1;
                BDD currentZero = current;

                while (true) {
                    BDD nextZero = vars->zeroBDD();
                    for (const auto & tr : trs.at(0)) {
                        nextZero = image(fw, currentZero, tr, solution_cost, g, 0, nextZero, max_nodes_limit, solvedWith);
                    }
                    zeroCostSteps++;
                    currentZero = nextZero;
                    if (currentZero.IsZero()) {
                        process_expanded(currentZero, g, zeroCostSteps);
                    } else {
                        break;                    
                    }
                }
            }

            for (const auto & trcost : trs){
                int c = trcost.first;
                if (c == 0 || g + c > solution_cost) continue;
                if (!open.count(g+c)) {
                    open[g+c] = vars->zeroBDD();
                }
                for (const auto & tr : trcost.second) {
                    open[g+c] = image(fw, current, tr, solution_cost, g, c, open[g+c], max_nodes_limit, solvedWith);
                }
            }
            open.erase(open.begin());
        }
    }
        
    BDD PlanReuseRegressionSearch::regress_plan_to_follower_initial_states (const FollowerSolution & sol,
                                                                            const BDD & follower_initial_states) {
        int cost = sol.solution_cost();

        std::shared_ptr<symbolic::ClosedList> closed_bw = sol.get_closed_bw();
        std::shared_ptr<symbolic::ClosedList> closed_fw = sol.get_closed_fw();
        
        std::map<int, BDD> solvedWith;

        BDD reached = vars->zeroBDD();
        for (const auto & entry :  closed_fw->getClosedList()) {
            reached += entry.second;            
            solvedWith[entry.first] = reached;
        }
        
        BDD cut_fw = sol.getCut();
        int cut_cost_fw = sol.getCutCost();

        cout << "regress_plan_to_follower_initial_states. cost " << cost << " cut cost: " << cut_cost_fw << endl;
        cout << "closed fw: ";
        
        closed_bw->insert(cost-cut_cost_fw, cut_fw);

        
        
        //Perform forward search 
        astar_perfect_search(true, sol.get_transition_relation(), cost, cut_fw, cut_cost_fw, closed_bw->getClosedList(), 0,
                             [&]  (const BDD & current, int g, int ) -> void {
                                 reached += current;
                                 solvedWith[g] = reached;
                             });

        BDD result = vars->zeroBDD();


        cout << "H: ";
        for (const auto & entry :  solvedWith) {
            cout << entry.first << " ";
            if (entry.second.IsZero()) {
                cout << "z" << " ";
            }
        }

        cout << endl;

        BDD goal = vars->getPartialStateBDD(g_goal);

        cout << closed_bw->getClosedList().at(0).nodeCount() << endl;
        cout << solvedWith[cost].nodeCount() << endl;
        cout << (solvedWith[cost]*closed_bw->getClosedList().at(0)).nodeCount() << endl;
        cout << goal.nodeCount() << endl;
        cout << (goal*closed_bw->getClosedList().at(0)).nodeCount() << endl;


        assert(!(goal*solvedWith[cost]).IsZero());

        //Perform backward search 
        astar_perfect_search(false, stackelberg_mgr->get_transition_relation(), cost, goal , 0, solvedWith , 0,
                             [&]  (const BDD & current, int g, int g_zero ) -> void {
                                 
                                 cout << "Closing " << g << " " <<                                      current.nodeCount() << endl;
                                 if (accumulate_intermediate_states || g == cost) {
                                     result += stackelberg_mgr->get_follower_initial_state_projection(current*stackelberg_mgr->get_static_follower_initial_state ());
                                 }

                                 closed_list_upper->insertWithZeroCostSteps(g, g_zero, current);
                             });

        assert(!result.IsZero());
      
        solvedFollowerInitialStates += result;
        return follower_initial_states - result;
    }


    PlanReuse *_parse_simple(OptionParser &parser) {
        parser.add_option<bool>("accumulate_intermediate_states", "Accumulate intermediate states in the plan.", "false");
    
        Options opts = parser.parse();
        if (!parser.dry_run()) {
            return new stackelberg::PlanReuseSimple(opts);
        }
        return NULL;
    }

    Plugin<PlanReuse> _plugin_plan_reuse_simple("simple", _parse_simple);


    

    PlanReuse *_parse_regress(OptionParser &parser) {
        parser.add_option<bool>("accumulate_intermediate_states", "Accumulate intermediate states in the plan.", "false");
        parser.add_option<int>("max_nodes_regression", "Maximum number of nodes to establish regression", "0");
    
        Options opts = parser.parse();
        if (!parser.dry_run()) {
            return new stackelberg::PlanReuseRegressionSearch(opts);
        }
        return NULL;
    }

    Plugin<PlanReuse> _plugin_plan_reuse_regress("regress", _parse_regress);
    
}














    
    // std::map<int, BDD>
    // SymbolicStackelbergManager::regress_plan(const std::vector<const GlobalOperator *> & plan) {
    //     std::map<int, BDD> result;
    //     int hstar = 0;
    //     BDD current = vars->getPartialStateBDD(g_goal);

    //     result[hstar] = current;

        
    //     for (int i = plan.size() -1; i >= 0; ++i ){
    //         const GlobalOperator *op = plan[i];
    //         int cost  =cost_type->get_adjusted_cost(op);
    //         hstar += cost;

    //         current = follower_transitions_by_id [op->get_op_id()]->preimage(current);
    //         if (cost == 0) {
    //             result[hstar] += current;
    //         } else {
    //             result[hstar] = current;
    //         }
    //     }
        
    //     return result;        
    // }


    // BDD SymbolicStackelbergManager::
    // regress_plan_to_follower_initial_states(const std::vector<const GlobalOperator *> & plan) {

    //     // std::map<int, std::vector <symbolic::TransitionRelation>> transitions;
    //     // for (int i = plan.size() -1; i >= 0; --i ){
    //     //     const GlobalOperator *op = plan[i];
    //     //     int cost  =cost_type->get_adjusted_cost(op);
            
    //     //     transitions[cost].push_back(TransitionRelation (vars.get(), op, cost));
    //     // }
        
    //     // for (map<int, vector<TransitionRelation>>::iterator it = transitions.begin();
    //     //      it != transitions.end(); ++it) {
    //     //     merge(vars.get(), it->second, mergeTR, mgr_params.max_tr_time, mgr_params.max_tr_size);
    //     // }

    //     // BDD current = vars->getPartialStateBDD(g_goal);
    //     // BDD result = current;
    //     // for (int i = plan.size() -1; i >= 0; --i ){
    //     //     const GlobalOperator *op = plan[i];
    //     //     int cost  =cost_type->get_adjusted_cost(op);
    //     //     BDD new_current = vars->zeroBDD();
    //     //     for (const auto & tr : transitions[cost]){
    //     //         new_current = tr.preimage(current);
    //     //     }
    //     //     current = new_current;
    //     //     result += current;

    //     // }        
    //     // return result;


    //     //TODO: This will fail if there is a goal on a variable that can only be touched
    //     // by the leader.
    //     BDD current = vars->getPartialStateBDD(g_goal);
    //     BDD result = current;

    //     for (int i = plan.size() -1; i >= 0; --i ){
    //         const GlobalOperator *op = plan[i];
    //         current = follower_transitions_by_id [op->get_op_id()]->preimage(current);
    //         result = current;
    //     }        
    //     return get_follower_initial_state_projection(result);        
    // }

