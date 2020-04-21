#include "stackelberg_bounded.h"

#include "util.h"

#include <vector>
#include "../option_parser.h"
#include "../plugin.h"
#include "../attack_success_prob_reuse_heuristic.h"
#include "../eager_search.h"
#include "../budget_dead_end_heuristic.h"

#include "partial_order_reduction.h"

#include <cassert>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include "../utils/timer.h"

#include "follower_search_engine.h"
#include "follower_task.h"


using namespace std;

namespace stackelberg {

    StackelbergBounded::StackelbergBounded(const Options &opts) :
        SearchEngine(opts),
        use_partial_order_reduction (opts.get<bool>("partial_order_reduction")),
        optimal_engine(opts.get<FollowerSearchEngine *>("optimal_engine")) {
    }

    void StackelbergBounded::initialize() {
        cout << "Initializing StackelbergBounded..." << endl;
        auto t1 = chrono::high_resolution_clock::now();

        task = make_unique<StackelbergTask> ();

        leader_vars_state_registry = task->get_leader_state_registry();
        
        follower_task_info.set_relevant_variables(task->get_leader_vars_follower_preconditioned());
        
        leader_operators_successor_generator.reset(
	    create_successor_generator(
		task->get_leader_variable_domain(), task->get_leader_operators(), task->get_leader_operators()));

        por = make_unique<PartialOrderReduction>(task.get());

        chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
        leader_search_initialize_duration = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();
    }

    SearchStatus StackelbergBounded::step() {
        cout << "Starting Stackelberg bounded search..." << endl;
        auto t1 = chrono::high_resolution_clock::now();

        int maxF = optimal_engine->solve_minimum_ftask();
        cout << "Solved minimum task: " << maxF << endl;
        
        int L = 0;
        int F = -1;
    
        while (!pareto_frontier.is_complete(maxF) && L < std::numeric_limits<int>::max()) {
            cout << "Leader action budget: " << L << endl;
            auto follower_tasks = gather_follower_tasks(L, F);
            cout << "There are " << follower_tasks.size()<< " follower tasks with L=" << L <<" F=" <<F << endl;
        
            //We do plan repair 
            if (plan_repair) {
                //Remove any task whose upper bound is below or equal to the best lower bound
                follower_tasks.erase(std::remove_if(follower_tasks.begin(), follower_tasks.end(),
                                                    [&](const auto * f_task) {
                                                        auto leader_state = leader_vars_state_registry->lookup_state(f_task->leader_state_id);
                                                        auto follower_state = task->get_follower_state(leader_state);
                                                        return plan_repair->solve(follower_state, F) <= F;}),
                                     follower_tasks.end());
            }

            // Among the remaining follower tasks, we need to sort them. We prefer to solve first 
            const auto f_cmp_tasks = [] (const FollowerTask * t1, const FollowerTask * t2) {
                if (t1->upper_bound > t2->upper_bound){
                    return true;
                } else if (t1->upper_bound == t2->upper_bound &&
                           t1->lower_bound >= t2->lower_bound) {
                    return true;
                }
                return false;
            };
            std::sort (follower_tasks.begin(), follower_tasks.end(), f_cmp_tasks);

            FollowerTask * best_f_task = nullptr;
            for (auto * f_task : follower_tasks) {
                if (f_task->upper_bound <= F) {
                    continue;
                }

                auto leader_state = leader_vars_state_registry->lookup_state(f_task->leader_state_id);
                auto follower_state = task->get_follower_state(leader_state);
                if (cost_bounded_engine) {
                    if (cost_bounded_engine->solve(follower_state, F)) {
                        num_follower_searches_cost_bounded++;
                        continue;
                    }
                }

                cout << "Solving leader state with L=" <<L  << " and F=" << F  << endl;

                int new_bound = optimal_engine->solve(follower_state, maxF);
                num_follower_searches++;
                num_follower_searches_optimal++;
                
                
                cout << "MY new bound is " << new_bound <<  " and the previous one is " << F << endl;
                if (new_bound > F) {
                    cout << "SET NEW BOUND" << endl;
                    F = new_bound;
                    best_f_task = f_task;
                }
            }

            if (best_f_task)  {
                pareto_frontier.add_node(L, F,
                                         best_f_task->get_leader_plan(),
                                         best_f_task->follower_plan);
            }

            pareto_frontier.set_complete_up_to(L);
            
            L = get_next_L();
        }

        auto t2 = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>( t2 - t1 ).count();

        cout << "total time: " << g_timer << endl;
        cout << "FixSearch initialize took: " << leader_search_initialize_duration << "ms" << endl;
        cout << "Complete Fixsearch took: " << duration << "ms" << endl;
        cout << "Search in Follower Statespace took " << (follower_search_duration_sum/1000) <<
            "ms" << endl;
        cout << "Search in Fixactions Statespace took " << (duration -
                                                            (follower_search_duration_sum/1000)) << "ms" << endl;
        cout << "reset_and_initialize_duration_sum: " << reset_and_initialize_duration_sum << "ms" << endl;

        cout << "Total follower searches " << num_follower_searches << endl;
        cout << "Optimal follower searches " << num_follower_searches_optimal << endl;
        cout << "Cost-bounded follower searches " << num_follower_searches_cost_bounded << endl;
        cout << "Plan-repair follower searches " << num_follower_searches_plan_repair << endl;

        pareto_frontier.dump(*task);
        exit(0);
        return IN_PROGRESS;
    }


    void StackelbergBoundedBFS::initialize() {
        StackelbergBounded::initialize();
        current_state = make_unique<GlobalState>(leader_vars_state_registry->get_initial_state());
        
        open_list = make_unique<StandardScalarOpenList<OpenListEntryLazy>>(&g_evaluator, false);
    }




//Gather all follower tasks that have a leader cost of exactly leader_cost. Any task whose
//follower_cost is higher or equal to follower_cost_bound can be ommited
    std::vector<FollowerTask *>
    StackelbergBoundedBFS::gather_follower_tasks(int leader_cost,
                                                 int /*follower_cost_bound*/) {

        std::vector<FollowerTask *> follower_tasks;
        assert (leader_cost == current_g);
    
        while (leader_cost == current_g) {
          //  cout << "Pop node with g=" << current_g <<  flush;
            SearchNode node = search_space.get_node(*current_state);
            bool reopen = (current_g < node.get_g()) && !node.is_dead_end() && !node.is_new();

          //  cout << " is new: " << node.is_new() << "   reopen " << reopen << endl;
            if (node.is_new() || reopen) {
                FollowerTask * task = &(follower_task_info[*current_state]);

                if (task->leader_cost > current_g) {
                    task->leader_cost = current_g;
                    task->leader_state_id = node.get_state_id();
                    follower_tasks.push_back(task);
                }

                follower_tasks.push_back(task);

                search_progress.inc_evaluated_states();
                open_list->evaluate(current_g, false);
                assert (!open_list->is_dead_end());  //This should not happen, since we are using g evaluator

                StateID dummy_id = current_predecessor_id;
                if (dummy_id != StateID::no_state) {
                    GlobalState parent_state = leader_vars_state_registry->lookup_state(dummy_id);
                    SearchNode parent_node = search_space.get_node(parent_state);

                    int h = 0;
                    if (reopen) {
                        node.reopen(parent_node, current_operator);
                        search_progress.inc_reopened();
                    } else {
                        node.open(h, parent_node, current_operator);
                    }
                }else {
               //     cout << "We had dummy_id" << endl;
                    node.open_initial(0);
                    search_progress.get_initial_h_values();
                }

                node.close();

                vector<const GlobalOperator *> operators;

              //  cout << "Successor generator" << endl;
                leader_operators_successor_generator->
                    generate_applicable_ops(*current_state, operators);

                // por->prune_applicable_leader_ops_sss(current_state, operators);
            
                search_progress.inc_generated(operators.size());

                for (size_t i = 0; i < operators.size(); ++i) {
                    int new_g = current_g + operators[i]->get_cost();
                    if (new_g < bound) {
                        open_list->evaluate(new_g, false);
                        open_list->insert(make_pair(current_state->get_id(), operators[i]));
                    }
                }

                search_progress.inc_expanded();
            }

            if(open_list->empty()) {
                current_g = std::numeric_limits<int>::max();
            } else {
                OpenListEntryLazy next = open_list->remove_min();
                current_predecessor_id = next.first;
                current_operator = next.second;
                GlobalState current_predecessor = leader_vars_state_registry->lookup_state(current_predecessor_id);
                assert(current_operator->is_applicable(current_predecessor));
                current_state = make_unique<GlobalState>(leader_vars_state_registry->get_successor_state(current_predecessor, *current_operator));

                SearchNode pred_node = search_space.get_node(current_predecessor);
                current_g = pred_node.get_real_g() + current_operator->get_cost();
            }

        }

        return follower_tasks;
    }


    StackelbergBoundedBFS::StackelbergBoundedBFS(const Options &opts) :
        StackelbergBounded (opts),
        current_predecessor_id(StateID::no_state),
        current_operator(nullptr),
        current_g(0) {

    }
    
    SearchEngine *_parse_bounded(OptionParser &parser) {
        SearchEngine::add_options_to_parser(parser);
    
        parser.add_option<FollowerSearchEngine *>("optimal_engine");
        parser.add_option<Heuristic *>("follower_heuristic", "The heuristic used for search in FollowerStateSpace", "", OptionFlags(false));
        parser.add_option<bool>("partial_order_reduction", "use partial order reduction for fix ops", "true");
    
        Options opts = parser.parse();
        if (!parser.dry_run()) {
            return new stackelberg::StackelbergBoundedBFS(opts);
        }
        return NULL;
    }

    Plugin<SearchEngine> _plugin_bounded("stackelberg_bounded", _parse_bounded);
}

