#include "plan_reuse.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "../global_operator.h"

#include "../symbolic/sym_variables.h"
#include "../symbolic/closed_list_disj.h"
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


    PlanReuse *_parse_simple(OptionParser &parser) {
        parser.add_option<bool>("accumulate_intermediate_states", "Accumulate intermediate states in the plan.", "false");
    
        Options opts = parser.parse();
        if (!parser.dry_run()) {
            return new stackelberg::PlanReuseSimple(opts);
        }
        return NULL;
    }

    Plugin<PlanReuse> _plugin_plan_reuse_simple("simple", _parse_simple);

    
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

