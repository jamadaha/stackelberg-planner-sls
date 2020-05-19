#include "symbolic_leader_search.h"

#include "util.h"

#include <vector>
#include "../option_parser.h"
#include "../plugin.h"

#include "../symbolic/uniform_cost_search.h"
#include "../symbolic/sym_controller.h"

#include "symbolic_stackelberg_manager.h"

#include <cassert>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include "../utils/timer.h"

#include "follower_search_engine.h"
#include "follower_task.h"
#include "plan_reuse.h"


using namespace std;
using namespace symbolic;

namespace stackelberg {

    SymbolicStackelberg::SymbolicStackelberg(const Options &opts) :
        SearchEngine(opts),
        optimal_engine(opts.get<FollowerSearchEngine *>("optimal_engine")),
        plan_reuse (opts.get<PlanReuse *>("plan_reuse")),
        mgrParams(opts), searchParams(opts) {

        task = make_unique<StackelbergTask> ();
        stackelberg_mgr = make_shared<SymbolicStackelbergManager> (task.get(), opts);
        optimal_engine->initialize(task.get(), stackelberg_mgr);

        plan_reuse->initialize(stackelberg_mgr);      

        if (opts.contains("satisficing_engine")) {
            satisficing_engine.reset(opts.get<FollowerSearchEngine *>("satisficing_engine"));
            satisficing_engine->initialize(task.get(), stackelberg_mgr);
        }

    }

    void SymbolicStackelberg::initialize() {
        cout << "Initializing SymbolicStackelberg..." << endl;
        auto t1 = chrono::high_resolution_clock::now();
        
        vars = stackelberg_mgr->get_sym_vars();

        leader_search_controller = make_unique<SymController> (vars, mgrParams, searchParams);
        leader_search = make_unique <UniformCostSearch> (leader_search_controller.get(), searchParams);
        auto mgr = stackelberg_mgr->get_leader_manager();
        
        leader_search->init(mgr, true);
        
        statistics.search_initialized(t1);
    }

    SearchStatus SymbolicStackelberg::step() {
        cout << "Starting Stackelberg bounded search..." << endl;
       
        int maxF = std::numeric_limits<int>::max();
        int L = 0;
        int F = -1;

        const auto & closed_list = leader_search->getClosed()->getClosedList();
        while (!pareto_frontier.is_complete(maxF)
               && L < std::numeric_limits<int>::max()
               && F < maxF) {
            BDD leader_states = closed_list.at(L);
            BDD follower_initial_states =
                stackelberg_mgr->get_follower_initial_state_projection(leader_states);
            
            cout << "L = " << L << ", leader states: " << vars->numStates(leader_states)
                 << ", follower subproblems: " << vars->numStates(follower_initial_states, stackelberg_mgr->get_num_follower_bdd_vars())  << ", " << flush;


            follower_initial_states =
                plan_reuse->find_plan_follower_initial_states(follower_initial_states);

            int newF = F;
            vector<int> current_best;
            FollowerSolution current_best_solution;
            while(!follower_initial_states.IsZero() && newF < maxF) {                
                auto state = stackelberg_mgr->sample_follower_initial_state(follower_initial_states);

                FollowerSolution solution; 
                if  (satisficing_engine) {
                    solution = satisficing_engine->solve(state, plan_reuse.get());                        
                }
  
                if (!solution.is_solved()) { 
                    solution = optimal_engine->solve(state, plan_reuse.get());
                }

                statistics.inc_opt_search();
                
                if (solution.has_plan()) {
                    const auto plan = solution.get_plan();
#ifndef NDEBUG
                    auto state_aux  = state;
                    for (auto * op : plan) {
                        assert(op->is_applicable(state_aux));
                        state_aux = op->apply_to(state_aux);
                        // cout << op->get_name() << endl;
                    }
#endif                
                    
                    follower_initial_states = plan_reuse->regress_plan_to_follower_initial_states(solution.get_plan(), follower_initial_states);

                    // BDD aux = vars->getPartialStateBDD(state,stackelberg_mgr->get_pattern_vars_follower_subproblems());
                    // assert(follower_initial_states-aux !=follower_initial_states);
                    // follower_initial_states -= aux;

                }
                
                int follower_cost  = solution.solution_cost();
       
                if (follower_cost > newF) {
                    newF = follower_cost;
                    current_best = state;
                    current_best_solution = solution;
                    plan_reuse->set_follower_bound(newF);
                }
            }


            if (newF > F) {
                F = newF;

                // cout << "Extract leader path" << endl;
                // cout << "leader state: " << endl; for (size_t i =0; i < g_fact_names.size(); ++i) { cout << g_fact_names[i][current_best[i]] << endl; }
 
                vector<const GlobalOperator *> leader_ops_sequence;                
                leader_search->getClosed()->extract_path(
                    vars->getPartialStateBDD(current_best,
                                             stackelberg_mgr->get_pattern_vars_follower_subproblems() ),
                    L, true, leader_ops_sequence);

                std::reverse(leader_ops_sequence.begin(), leader_ops_sequence.end());
                
                pareto_frontier.add_node(L, F, leader_ops_sequence, current_best_solution.get_plan());
                
           }

            cout << "F: " << F << ", total time: " << g_timer() <<  endl;

            //Generate the next layer
            while(!leader_search->finished() && leader_search->getG() == L) {
                leader_search->step();
            }

            assert (leader_search->getG() > L);
            L = leader_search->getG();   
        }

        statistics.search_finished();
        BDD followerStates = stackelberg_mgr->get_follower_initial_state_projection(leader_search->getClosedTotal());
        cout << "Total number of leader states: " << vars->numStates(leader_search->getClosedTotal()) << endl;
        cout << "Total number of follower subproblems: " << vars->numStates(followerStates, stackelberg_mgr->get_num_follower_bdd_vars()) << endl;

        statistics.dump();
        pareto_frontier.dump(*task);

        exit(0);

        return IN_PROGRESS;
    }
    
    SearchEngine *_parse_ss(OptionParser &parser) {
        SearchEngine::add_options_to_parser(parser);
        SymbolicStackelbergManager::add_options_to_parser(parser);
        SymParamsSearch::add_options_to_parser(parser, 30e3, 10e7);
        
        parser.add_option<bool>("project_to_follower_states", "Project the set of leader options to follower states.", "false");

        parser.add_option<PlanReuse *>("plan_reuse", "strategy for plan reuse", "simple");
        parser.add_option<FollowerSearchEngine *>("optimal_engine");
        parser.add_option<FollowerSearchEngine *>("satisficing_engine", "", "", 
        OptionFlags(false));
    
        Options opts = parser.parse();
        if (!parser.dry_run()) {
            return new stackelberg::SymbolicStackelberg(opts);
        }
        return NULL;
    }

    Plugin<SearchEngine> _plugin_sym_leader_search("sym_stackelberg", _parse_ss);
}

