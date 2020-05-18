#include "follower_search_engine.h"

#include "../option_parser.h"
#include "../plugin.h"
#include "../successor_generator.h"
#include "../search_engine.h"

#include "util.h"
#include "stackelberg_task.h"
#include "symbolic_stackelberg_manager.h"
#include "plan_reuse.h"
#include "../symbolic/sym_search.h"
#include "../symbolic/sym_variables.h"
#include "../symbolic/sym_params_search.h"
#include "../symbolic/original_state_space.h"
#include "../symbolic/uniform_cost_search.h"
#include "../symbolic/bidirectional_search.h"
#include "../symbolic/sym_controller.h"



using namespace stackelberg;
using namespace symbolic;


        
FollowerSolution::FollowerSolution(const SymSolution & sol, const  vector<int> & initial_state, const vector<bool> & pattern) : solved(true), plan_cost(sol.getCost()) {

    sol.getPlan(plan, initial_state, pattern);
}
        

FollowerSolution SymbolicFollowerSearchEngine::solve (const std::vector<int> & leader_state,
                                                      PlanReuse * plan_reuse ) {

    auto controller = make_unique<SymController> (vars, mgrParams, searchParams);

    auto mgr = stackelberg_mgr->get_follower_manager(leader_state);

    auto fw_search = make_unique <UniformCostSearch> (controller.get(), searchParams);
    
    unique_ptr<BidirectionalSearch> bd_search;
    SymSearch * search;

    // if(plan_reuse) {
    //     fw_search->init(mgr, true, plan_reuse->getClosed());
    //     search = fw_search.get();
    // } else
    if (bidir) {
        auto bw_search = make_unique <UniformCostSearch> (controller.get(), searchParams);


        // if (plan_reuse) {
        //     fw_search->init(mgr, true, make_shared<OppositeFrontierComposite>(bw_search->getClosedShared(), plan_reuse->get_closed()));
        // } else {
            fw_search->init(mgr, true, bw_search->getClosedShared());
            //}
        bw_search->init(mgr, false, fw_search->getClosedShared());	
        bd_search = make_unique<BidirectionalSearch> (controller.get(), searchParams,
                                                      move(fw_search), move(bw_search));
        search = bd_search.get();
    }else{
        fw_search->init(mgr, true);
        search = fw_search.get();
    }

    while(!controller->solved()) {
        if (plan_reuse && controller->getUpperBound() <= plan_reuse->get_follower_bound()) {
            return FollowerSolution(*(controller->get_solution()), leader_state, mgr->get_relevant_vars());
        }
        
        search->step();
    }

   
    if (controller->getUpperBound() < std::numeric_limits<int>::max()) {
        assert(controller->solved());
        return FollowerSolution(*(controller->get_solution()), leader_state, mgr->get_relevant_vars());
    } else {
        return FollowerSolution(controller->getUpperBound());
    }
}

FollowerSolution SymbolicFollowerSearchEngine::solve_minimum_ftask () {

    // auto controller = make_unique<SymController> (vars, mgrParams, searchParams);
    // auto fw_search = make_unique <UniformCostSearch> (controller.get(), searchParams);
    // auto bw_search = make_unique <UniformCostSearch> (controller.get(), searchParams);

    // auto mgr = make_shared<SymbolicStackelbergManager> (vars.get(),
    //                                                     mgrParams,
    //                                                     OperatorCostFunction::get_cost_function());
    
    // fw_search->init(mgr, true, bw_search->getClosedShared());
    // bw_search->init(mgr, false, fw_search->getClosedShared());
	
    // auto search = make_unique<BidirectionalSearch> (controller.get(),
    //                                                 searchParams,
    //                                                 move(fw_search),
    //                                                 move(bw_search));

    // while(!search->finished()) {
    //     search->step();
    // }

    // cout << "Follower Search finished: " << search->finished() << endl;
    // cout << "Controller upper bound: " << controller->getUpperBound() << endl;
    
    // return controller->getUpperBound();

    return FollowerSolution();
}

void ExplicitFollowerSearchEngine::initialize_follower_search_engine() {
}

ExplicitFollowerSearchEngine::ExplicitFollowerSearchEngine(const Options &opts) :
    search_engine (opts.get<SearchEngine *>("search_engine"))  {
    if (opts.contains("follower_heuristic")) {
        follower_heuristic =
            opts.get<Heuristic *>("follower_heuristic"); 
    } else {
        follower_heuristic = nullptr;
    }
}


FollowerSolution ExplicitFollowerSearchEngine::solve_minimum_ftask () {
    g_operators.clear();
    task->compute_always_applicable_follower_ops(g_operators);
    cout << "number of always applicable attack ops: " << g_operators.size() << endl;
    delete g_successor_generator;
    g_successor_generator = create_successor_generator(g_variable_domain, g_operators, g_operators);
    search_engine->reset();
    g_state_registry->reset();
    if(follower_heuristic) {
        follower_heuristic->reset();
    }

    int follower_cost_upper_bound = StackelbergTask::FOLLOWER_TASK_UNSOLVABLE;
    search_engine->search();
    if (search_engine->found_solution()) {
        search_engine->save_plan_if_necessary();
        follower_cost_upper_bound = search_engine->calculate_plan_cost();
    }
    cout << "follower_cost_upper_bound: " << follower_cost_upper_bound << endl;

    g_operators.clear();
    for (const auto & op :task->get_follower_operators_with_all_preconds()) {
        g_operators.push_back(op);
    }
    delete g_successor_generator;
    g_successor_generator = create_successor_generator(g_variable_domain, g_operators, g_operators);
    if(follower_heuristic) {
        follower_heuristic->reset();
    
    }
    return FollowerSolution(); //follower_cost_upper_bound;
}


FollowerSolution ExplicitFollowerSearchEngine::solve (const std::vector<int> & leader_state, PlanReuse * /*desired_bound*/) {
    for (int leader_var = 0; leader_var < task->get_num_leader_vars(); leader_var++) {
        int orig_var_id = task-> get_map_leader_var_id_to_orig_var_id(leader_var);
        g_initial_state_data[orig_var_id] = leader_state[leader_var];
    }

    search_engine->reset();
    g_state_registry->reset();
    
    search_engine->search(); 

    // SearchSpace *search_space = search_engine->get_search_space();
    // all_attacker_states += search_space->get_num_search_node_infos();

    // // int plan_cost = StackelbergTask::FOLLOWER_TASK_UNSOLVABLE; 
    // if (search_engine->found_solution()) {
    //     search_engine->save_plan_if_necessary();
    //     plan_cost = search_engine->calculate_plan_cost();

    //     // if (follower_heuristic) {
    //     //     follower_heuristic_search_space = new AttackSearchSpace();
    //     //     free_follower_heuristic_per_state_info = true;
                
    //     //     OpenList<pair<StateID, int>> *open_list = ((EagerSearch *)
    //     //                                                search_engine)->get_open_list();
    //     //     const GlobalState *goal_state = search_engine->get_goal_state();
    //     //     const int goal_state_budget = search_engine->get_goal_state_budget();

    //     //     follower_heuristic_search_space->budget_attack_search_node_infos.set_relevant_variables(attack_vars_indizes);

    //     //     follower_heuristic->reinitialize(follower_heuristic_search_space, search_space,
    //     //                                      open_list, *goal_state,
    //     //                                      goal_state_budget);
    //     // }
    // }
    
    return FollowerSolution(); //plan_cost;
}



SymbolicFollowerSearchEngine::SymbolicFollowerSearchEngine(const Options &opts) :
    mgrParams(opts), searchParams(opts), bidir(opts.get<bool>("bidir")) {
}

void SymbolicFollowerSearchEngine::initialize_follower_search_engine() {
    vars = stackelberg_mgr->get_sym_vars();
}


static FollowerSearchEngine *_parse_symbolic(OptionParser &parser) {
   
    // SearchEngine::add_options_to_parser(parser);
    SymVariables::add_options_to_parser(parser);
    SymParamsSearch::add_options_to_parser(parser, 30e3, 10e7);
    SymParamsMgr::add_options_to_parser(parser);

    parser.add_option<bool> ("bidir", "Use bidirectional search", "true");

    
    Options opts = parser.parse();


    if (!parser.dry_run()) {
        return new SymbolicFollowerSearchEngine(opts);
    }

    return nullptr;
}

static Plugin<FollowerSearchEngine> _plugin_symbolic("symbolic", _parse_symbolic);
