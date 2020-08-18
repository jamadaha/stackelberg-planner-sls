#include "follower_search_engine.h"

#include "../option_parser.h"
#include "../plugin.h"
#include "../successor_generator.h"
#include "../search_engine.h"

#include <chrono>

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

using namespace symbolic;

namespace stackelberg {
    FollowerSearchEngine::FollowerSearchEngine(const Options &opts)  :
        plan_reuse_upper_bound (opts.get<bool> ("plan_reuse_upper_bound")),
        time_limit_seconds_minimum_task(opts.get<int>("time_limit_seconds_minimum_task")) {
    }

    void FollowerSearchEngine::add_options_to_parser(OptionParser &parser) {   
        parser.add_option<bool> ("plan_reuse_upper_bound", "reuse upper bound computed from previous plans", "true");

        parser.add_option<int> ("time_limit_seconds_minimum_task",
                                "time limit for solving the minimum ftask in seconds", "10000");    

    }


    FollowerSolution::FollowerSolution(const SymSolution & sol, const  vector<int> & initial_state, const vector<bool> & pattern,  int lb,
                                       std::shared_ptr<symbolic::ClosedList> _closed_fw,
                                       std::shared_ptr<symbolic::ClosedList> _closed_bw) : solved(true), plan_cost(sol.getCost()),
                                                                                           lower_bound(lb), closed_fw(_closed_fw),
                                                                                           closed_bw(_closed_bw), cut(sol.getCut()),
                                                                                           cut_cost(sol.getCutCostFw()), trs(sol.get_transition_relation()) {

        sol.getPlan(plan, initial_state, pattern);
    }
        

    FollowerSolution SymbolicFollowerSearchEngine::solve (const std::vector<int> & leader_state,
                                                          PlanReuse * plan_reuse, int /*bound*/ ) {

        auto controller = make_unique<SymController> (vars, mgrParams, searchParams);

        auto mgr = stackelberg_mgr->get_follower_manager(leader_state);

        auto fw_search = make_unique <UniformCostSearch> (controller.get(), searchParams);
        auto fw_search_closed = fw_search->getClosedShared();
        unique_ptr<BidirectionalSearch> bd_search;
        SymSearch * search;

        shared_ptr<ClosedList> bw_search_closed;
        // if(plan_reuse) {
        //     fw_search->init(mgr, true, plan_reuse->get_closed());
        //     search = fw_search.get();
        // } else  
        if (bidir) {
            auto bw_search = make_unique <UniformCostSearch> (controller.get(), searchParams);

            if (plan_reuse && plan_reuse_upper_bound) {
                fw_search->init(mgr, true, make_shared<OppositeFrontierComposite>(bw_search->getClosedShared(),
                                                                                  plan_reuse-> get_opposite_frontier (leader_state)));
            } else {
                fw_search->init(mgr, true, bw_search->getClosedShared());
            }
            bw_search->init(mgr, false, fw_search->getClosedShared());
                        bw_search_closed = bw_search->getClosedShared();
            bd_search = make_unique<BidirectionalSearch> (controller.get(), searchParams,
                                                          move(fw_search), move(bw_search));
            search = bd_search.get();

        }else{
            fw_search->init(mgr, true);
            search = fw_search.get();
        }

        while(!controller->solved()) {
            if (plan_reuse && controller->getUpperBound() <= plan_reuse->get_follower_bound()) {
                return FollowerSolution(*(controller->get_solution()),
                                        leader_state, mgr->get_relevant_vars(),
                                        controller->getLowerBound(), fw_search_closed, bw_search_closed);
            }
        
            search->step();
        }

   
        if (controller->getUpperBound() < std::numeric_limits<int>::max()) {
            assert(controller->solved());
            return FollowerSolution(*(controller->get_solution()),
                                    leader_state, mgr->get_relevant_vars(),
                                    controller->getLowerBound(), fw_search_closed, bw_search_closed);
        } else {
            return FollowerSolution(controller->getUpperBound(), controller->getLowerBound());
        }
    }

    FollowerSolution SymbolicFollowerSearchEngine::solve_minimum_ftask (PlanReuse * plan_reuse) {

        auto t1 = chrono::high_resolution_clock::now();
                        
        auto controller = make_unique<SymController> (vars, mgrParams, searchParams);

        auto mgr = stackelberg_mgr->get_follower_manager_minimal();

        auto fw_search = make_unique <UniformCostSearch> (controller.get(), searchParams);
        auto fw_search_closed = fw_search->getClosedShared();

        unique_ptr<BidirectionalSearch> bd_search;

        shared_ptr<ClosedList> bw_search_closed;
        SymSearch * search;
        UniformCostSearch * bw_search_ptr = nullptr;
        if (bidir) {
            auto bw_search = make_unique <UniformCostSearch> (controller.get(), searchParams);
            bw_search_ptr = bw_search.get();
            fw_search->init(mgr, true, bw_search->getClosedShared());
            
            bw_search->init(mgr, false, fw_search->getClosedShared());

            if (force_bw_search_minimum_task_seconds) {

                while(!bw_search->finished() && chrono::duration_cast<chrono::seconds>(chrono::high_resolution_clock::now() - t1).count() <
                      force_bw_search_minimum_task_seconds) {
                    bw_search->step();
                }
            }

            bw_search_closed = bw_search->getClosedShared();
            bd_search = make_unique<BidirectionalSearch> (controller.get(), searchParams,
                                                          move(fw_search), move(bw_search));
            search = bd_search.get();


        }else{
            fw_search->init(mgr, true);
            search = fw_search.get();
        }

        while(!controller->solved()  && chrono::duration_cast<chrono::seconds>(chrono::high_resolution_clock::now() - t1).count() < time_limit_seconds_minimum_task) {
            search->step();
        }


        if(plan_reuse && plan_reuse_minimal_task_upper_bound && bw_search_ptr) {
            plan_reuse->load_plans(*(bw_search_ptr->getClosed()));
        }
   
        if (controller->getUpperBound() < std::numeric_limits<int>::max()) {
            cout << "Max upper bound: " << controller->getUpperBound() << endl;
            assert(controller->solved());
            return FollowerSolution(*(controller->get_solution()), g_initial_state_data, mgr->get_relevant_vars(), controller->getLowerBound(),  fw_search_closed, bw_search_closed);
        } else {
                cout << "Max upper bound: unsolvable " << endl;
                return FollowerSolution(controller->getUpperBound(), controller->getLowerBound());
        }
    }

    void ExplicitFollowerSearchEngine::initialize_follower_search_engine() {
        follower_operators_with_all_preconds = task->get_follower_operators_with_all_preconds();
        successor_generator.reset(create_successor_generator(g_variable_domain, follower_operators_with_all_preconds, follower_operators_with_all_preconds));

    }

    ExplicitFollowerSearchEngine::ExplicitFollowerSearchEngine(const Options &opts) : FollowerSearchEngine(opts), 
                                                                                      search_engine (opts.get<SearchEngine *>("search_engine")),
                                                                                      is_optimal_solver(opts.get<bool>("is_optimal_solver")),
                                                                                      debug(opts.get<bool>("debug")) {
        if (opts.contains("follower_heuristic")) {
            follower_heuristic =
                opts.get<Heuristic *>("follower_heuristic"); 
        } else {
            follower_heuristic = nullptr;
        }
    }


    FollowerSolution ExplicitFollowerSearchEngine::solve_minimum_ftask (PlanReuse * /*plan_reuse*/) {
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

    FollowerSolution ExplicitFollowerSearchEngine::solve (const std::vector<int> & leader_state, PlanReuse * plan_reuse, int bound) {
        // Save global information
        auto * g_successor_generator_copy = g_successor_generator;
        auto original_g_initial_state_data = g_initial_state_data;

        //Overwritte global information
        g_operators.swap(follower_operators_with_all_preconds);
        g_successor_generator = successor_generator.get();
        g_initial_state_data = leader_state;

        search_engine->reset();
        g_state_registry->reset();

        streambuf *old = nullptr;
        if (!debug) {
            old = cout.rdbuf(); // <-- save
            stringstream ss;
            cout.rdbuf(ss.rdbuf());        // <-- redirect
        }
        search_engine->set_bound(bound);

        if(plan_reuse && plan_reuse_upper_bound) {
            search_engine->set_opposite_frontier(plan_reuse->get_opposite_frontier_explicit(leader_state));
        }

        
        search_engine->search();

        follower_statistics.accumulate(search_engine->get_search_progress());  

        
        // SearchSpace *search_space = search_engine->get_search_space();
        // all_attacker_states += search_space->get_num_search_node_infos();

        int plan_cost = std::numeric_limits<int>::max();

        std::vector <const GlobalOperator *> plan;

        if (search_engine->found_solution()) {
            plan_cost = search_engine->calculate_plan_cost();
            plan = search_engine->get_plan();
        } 


        if (!debug) {
            cout.rdbuf(old);
        }

        search_engine->set_bound(std::numeric_limits<int>::max()); // restore bound.
        
        //Restore globals
        g_initial_state_data = original_g_initial_state_data;
        g_operators.swap(follower_operators_with_all_preconds);
        g_successor_generator = g_successor_generator_copy;

        if (search_engine->get_status() == TIMEOUT) {
            return FollowerSolution();
        }

        if(is_optimal_solver) {
            return FollowerSolution(plan_cost, plan, search_engine->get_search_progress().get_f_value());
        }else {
            assert(!FollowerSolution(plan_cost, plan).is_optimal());
            return FollowerSolution(plan_cost, plan);
        }
    }



    SymbolicFollowerSearchEngine::SymbolicFollowerSearchEngine(const Options &opts) :
        FollowerSearchEngine(opts), 
        mgrParams(opts), searchParams(opts), bidir(opts.get<bool>("bidir")),
        plan_reuse_minimal_task_upper_bound(opts.get<bool>("plan_reuse_minimal_task_upper_bound")),
        force_bw_search_minimum_task_seconds (opts.get<int>("force_bw_search_minimum_task_seconds")) {
    }

    void SymbolicFollowerSearchEngine::initialize_follower_search_engine() {
        vars = stackelberg_mgr->get_sym_vars();
    }

}
static stackelberg::FollowerSearchEngine *_parse_symbolic(OptionParser &parser) {
   
    // SearchEngine::add_options_to_parser(parser);
    SymVariables::add_options_to_parser(parser);
    SymParamsSearch::add_options_to_parser(parser, 30e3, 10e7);
    SymParamsMgr::add_options_to_parser(parser);
    stackelberg::FollowerSearchEngine::add_options_to_parser(parser);

    parser.add_option<bool> ("bidir", "Use bidirectional search", "true");
    parser.add_option<bool> ("plan_reuse_minimal_task_upper_bound",
                             "Reuse all backward search for the minimal search for plan reuse purposes", "true");    


    parser.add_option<int> ("force_bw_search_minimum_task_seconds",
                             "Perform backward search on the minimum task for at least that many seconds", "0");    

  
    Options opts = parser.parse();


    if (!parser.dry_run()) {
        return new stackelberg::SymbolicFollowerSearchEngine(opts);
    }

    return nullptr;
}




static stackelberg::FollowerSearchEngine *_parse_explicit(OptionParser &parser) {
    stackelberg::FollowerSearchEngine::add_options_to_parser(parser);
    parser.add_option<SearchEngine *> ("search_engine", "engine", "");
    parser.add_option<bool> ("is_optimal_solver", "engine", "");
    parser.add_option<bool> ("debug", "show output of follower search ", "false");
        
    Options opts = parser.parse();

    if (!parser.dry_run()) {
        return new stackelberg::ExplicitFollowerSearchEngine(opts);
    }

    return nullptr;
}


static Plugin<stackelberg::FollowerSearchEngine> _plugin_symbolic_follower("symbolic", _parse_symbolic);
static Plugin<stackelberg::FollowerSearchEngine> _plugin_explicit_follower("explicit", _parse_explicit);
