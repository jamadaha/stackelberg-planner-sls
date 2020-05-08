#ifndef STACKELBERG_STATISTICS_H
#define STACKELBERG_STATISTICS_H

#include <algorithm>
#include <chrono>

namespace stackelberg {
    class StackelbergStatistics {

        long int time_search_initialization;
        
        int num_follower_searches = 0;
        int num_follower_searches_optimal = 0;
        int num_follower_searches_cost_bounded = 0;
        int num_follower_searches_plan_repair = 0;
        
        long follower_search_duration_sum = 0;
        long reset_and_initialize_duration_sum = 0;
        long leader_search_initialize_duration = 0;
        int all_follower_states = 0;
        int num_leader_op_paths = 0;


    public:
        void dump() const;

        void search_initialized(std::chrono::high_resolution_clock::time_point t1);
    
    };

}

#endif
