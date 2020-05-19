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
    follower_operators_with_all_preconds = task->get_follower_operators_with_all_preconds();
    successor_generator.reset(create_successor_generator(g_variable_domain, follower_operators_with_all_preconds, follower_operators_with_all_preconds));

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
    // auto * g_successor_generator_copy = g_successor_generator;
    // g_successor_generator = successor_generator;
    // g_operators.clear();
    
    // task->compute_always_applicable_follower_ops(g_operators);
    // cout << "number of always applicable attack ops: " << g_operators.size() << endl;
    // delete g_successor_generator;

    // search_engine->reset();
    // g_state_registry->reset();
    // if(follower_heuristic) {
    //     follower_heuristic->reset();
    // }

    // int follower_cost_upper_bound = StackelbergTask::FOLLOWER_TASK_UNSOLVABLE;
    // search_engine->search();
    // if (search_engine->found_solution()) {
    //     search_engine->save_plan_if_necessary();
    //     follower_cost_upper_bound = search_engine->calculate_plan_cost();
    // }
    // cout << "follower_cost_upper_bound: " << follower_cost_upper_bound << endl;

    // g_operators.clear();
    // for (const auto & op :task->get_follower_operators_with_all_preconds()) {
    //     g_operators.push_back(op);
    // }
    // delete g_successor_generator;
    // g_successor_generator = create_successor_generator(g_variable_domain, g_operators, g_operators);
    // if(follower_heuristic) {
    //     follower_heuristic->reset();    
    // }
    return FollowerSolution(); //follower_cost_upper_bound;
}


FollowerSolution ExplicitFollowerSearchEngine::solve (const std::vector<int> & leader_state, PlanReuse * /*desired_bound*/) {
    // Save global information
    auto * g_successor_generator_copy = g_successor_generator;
    auto original_g_initial_state_data = g_initial_state_data;

    //Overwritte global information
    g_operators.swap(follower_operators_with_all_preconds);
    g_successor_generator = successor_generator.get();
    g_initial_state_data = leader_state;

    search_engine->reset();
    g_state_registry->reset();

#ifdef NDEBUG
    streambuf *old = cout.rdbuf(); // <-- save
    stringstream ss;
    cout.rdbuf(ss.rdbuf());        // <-- redirect
#endif   
    
    search_engine->search(); 

    // SearchSpace *search_space = search_engine->get_search_space();
    // all_attacker_states += search_space->get_num_search_node_infos();

    int plan_cost = std::numeric_limits<int>::max();

    std::vector <const GlobalOperator *> plan;

    if (search_engine->found_solution()) {
        plan_cost = search_engine->calculate_plan_cost();
        plan = search_engine->get_plan();
    } 


#ifdef NDEBUG
    cout.rdbuf(old);   			// <-- restore
#endif

    //Restore globals
    g_initial_state_data = original_g_initial_state_data;
    g_operators.swap(follower_operators_with_all_preconds);
    g_successor_generator = g_successor_generator_copy;

    return FollowerSolution(plan_cost, plan);
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




static FollowerSearchEngine *_parse_explicit(OptionParser &parser) {

    parser.add_option<SearchEngine *> ("search_engine", "engine", "");
    Options opts = parser.parse();

    if (!parser.dry_run()) {
        return new ExplicitFollowerSearchEngine(opts);
    }

    return nullptr;
}


static Plugin<FollowerSearchEngine> _plugin_symbolic_follower("symbolic", _parse_symbolic);
static Plugin<FollowerSearchEngine> _plugin_explicit_follower("explicit", _parse_explicit);
