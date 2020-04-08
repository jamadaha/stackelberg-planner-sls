#include "follower_search_engine.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "symbolic_stackelberg_manager.h"
#include "../symbolic/sym_search.h"
#include "../symbolic/sym_variables.h"
#include "../symbolic/sym_params_search.h"
#include "../symbolic/original_state_space.h"
#include "../symbolic/uniform_cost_search.h"
#include "../symbolic/bidirectional_search.h"
#include "../symbolic/sym_controller.h"



using namespace stackelberg;
using namespace symbolic;

int SymbolicFollowerSearchEngine::solve (const FollowerTask & ftask, int desired_bound) {

    auto mgr = make_shared<SymbolicStackelbergManager> (vars.get(), mgrParams, OperatorCostFunction::get_cost_function(), ftask);

    auto controller = make_unique<SymController> (vars, mgrParams, searchParams);
    
    auto fw_search = make_unique <UniformCostSearch> (controller.get(), searchParams);
    auto bw_search = make_unique <UniformCostSearch> (controller.get(), searchParams);
    fw_search->init(mgr, true, bw_search->getClosedShared());
    bw_search->init(mgr, false, fw_search->getClosedShared());
	
    auto search = make_unique<BidirectionalSearch> (controller.get(), searchParams, move(fw_search), move(bw_search));

    while(!search->finished() && controller->getUpperBound() > desired_bound) {
        search->step();
    }

    cout << "Search finished: " << search->finished() << endl;
    cout << "Controller upper bound: " << controller->getUpperBound() << endl;
    cout << "Desired bound: " << desired_bound << endl;
    
    return controller->getUpperBound();
}

int SymbolicFollowerSearchEngine::solve_minimum_ftask () {
    return std::numeric_limits<int>::max();
}


// int ExplicitFollowerSearchEngine::solve_minimum_ftask () {
//     g_operators.clear();
//     task->compute_always_applicable_follower_ops(g_operators);
//     cout << "number of always applicable attack ops: " << g_operators.size() << endl;
//     delete g_successor_generator;
//     g_successor_generator = create_successor_generator(g_variable_domain, g_operators, g_operators);
//     search_engine->reset();
//     g_state_registry->reset();
//     if(follower_heuristic) {
//         follower_heuristic->reset();
//     }
            
//     search_engine->search();
//     if (search_engine->found_solution()) {
//         search_engine->save_plan_if_necessary();
//         follower_cost_upper_bound = search_engine->calculate_plan_cost();
//     } else {
//         follower_cost_upper_bound = StackelbergTask::FOLLOWER_TASK_UNSOLVABLE;
//     }
//     cout << "follower_cost_upper_bound: " << follower_cost_upper_bound << endl;


//     g_operators.clear();
//     for (const auto & op :task->get_follower_operators_with_all_preconds()) {
//         g_operators.push_back(op);
//     }
//     delete g_successor_generator;
//     g_successor_generator = create_successor_generator(g_variable_domain, g_operators, g_operators);
//     if(follower_heuristic) {
//         follower_heuristic->reset();
//     }

//     search_engine->
//         }



SymbolicFollowerSearchEngine::SymbolicFollowerSearchEngine(const Options &opts) :
    vars(make_shared<SymVariables>(opts)), mgrParams(opts), searchParams(opts) {
    vars->init();

}

static FollowerSearchEngine *_parse_symbolic(OptionParser &parser) {
   
    // SearchEngine::add_options_to_parser(parser);
    SymVariables::add_options_to_parser(parser);
    SymParamsSearch::add_options_to_parser(parser, 30e3, 10e7);
    SymParamsMgr::add_options_to_parser(parser);
    
    Options opts = parser.parse();


    if (!parser.dry_run()) {
        return new SymbolicFollowerSearchEngine(opts);
    }

    return nullptr;
}

static Plugin<FollowerSearchEngine> _plugin_symbolic("symbolic", _parse_symbolic);
