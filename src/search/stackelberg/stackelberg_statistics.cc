#include "stackelberg_statistics.h"

#include "../globals.h"
#include <iostream>

using namespace std;

namespace stackelberg {

    StackelbergStatistics::StackelbergStatistics() : time_optimal_solver(0),
                                                     time_cost_bounded_solver(0),
                                                     time_solvers(0) {}


    void StackelbergStatistics::dump() const {

        // auto t2 = chrono::high_resolution_clock::now();
        // auto duration = chrono::duration_cast<chrono::microseconds>( t2 - t1 ).count();


        
        cout <<  "Total follower searches: " << num_follower_searches << endl;
        cout << "Solved by optimal solver: " << num_follower_searches_optimal_solver << endl;
        cout << "Solved by cost bounded solver: " << num_follower_searches_cost_bounded_solver << endl;
        cout << "Optimally solved follower subproblems: " << num_follower_searches_optimal_solution << endl;

        cout << "Optimal solver time: " << (time_optimal_solver.count()/1000.0) << "s" << endl;
        cout << "Cost-bounded solver time: " << (time_cost_bounded_solver.count()/1000.0) << "s" << endl;
        cout << "Follower search time: " << (time_solvers.count()/1000.0) << "s" <<  endl;
        
        cout << "Total time: " << g_timer << endl;

        // cout << "Total follower searches " << num_follower_searches << endl;
        //     cout << "Optimal follower searches " << num_follower_searches_optimal << endl;
        //     cout << "Cost-bounded follower searches " << num_follower_searches_cost_bounded << endl;
        //     cout << "Plan-repair follower searches " << num_follower_searches_plan_repair << endl;
    }


    void StackelbergStatistics::stackelberg_search_initialized(std::chrono::high_resolution_clock::time_point t1) {
        time_begin_stackelberg_search = chrono::high_resolution_clock::now();
        long time_search_initialization = chrono::duration_cast<chrono::milliseconds>(time_begin_stackelberg_search - t1).count();
        cout << "Search initialized: " << time_search_initialization/1000.0 << "s" << endl;
    }


    void StackelbergStatistics::stackelberg_search_finished() {
        auto t2 = chrono::high_resolution_clock::now();
        long time_search = chrono::duration_cast<chrono::milliseconds>(t2 - time_begin_stackelberg_search).count();
        cout << "Search time: " << time_search/1000.0 << "s" << endl;
    }


    void StackelbergStatistics::follower_search_finished (std::chrono::milliseconds runtime,
                                                          bool optimal_solver,
                                                          bool optimal_solution) {
        num_follower_searches++;
        time_solvers += runtime;
        if (optimal_solver) {
            time_optimal_solver += runtime;
            num_follower_searches_optimal_solver ++;
        } else {
            time_cost_bounded_solver += runtime;
            num_follower_searches_cost_bounded_solver ++;
        }

        if (optimal_solution) {
            num_follower_searches_optimal_solution ++;
        }
    }


}
