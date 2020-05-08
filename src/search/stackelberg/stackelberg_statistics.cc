#include "stackelberg_statistics.h"

#include <iostream>

using namespace std;

namespace stackelberg {
    
    void StackelbergStatistics::dump() const {

        // auto t2 = chrono::high_resolution_clock::now();
        // auto duration = chrono::duration_cast<chrono::microseconds>( t2 - t1 ).count();

    
        // cout << "Optimally solved follower subproblems: " << num_follower_searches << endl;
        // cout << "Search time: " << (duration/1000000.0) << "s" << endl;
        // cout << "Follower search time: " << (follower_search_duration_sum/1000000.0) << "s" << endl;
        // cout << "Leader search time: " << ((duration-follower_search_duration_sum)/1000000.0) << "s" << endl;
        // cout << "Total time: " << g_timer << endl;

        // cout << "Total follower searches " << num_follower_searches << endl;
        //     cout << "Optimal follower searches " << num_follower_searches_optimal << endl;
        //     cout << "Cost-bounded follower searches " << num_follower_searches_cost_bounded << endl;
        //     cout << "Plan-repair follower searches " << num_follower_searches_plan_repair << endl;
    }


    void StackelbergStatistics::search_initialized(std::chrono::high_resolution_clock::time_point t1) {
        auto t2 = chrono::high_resolution_clock::now();
        time_search_initialization = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();
        cout << "Search initialized: " << time_search_initialization/1000.0 << "s" << endl;
    }

}
