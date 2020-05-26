#ifndef STACKELBERG_STATISTICS_H
#define STACKELBERG_STATISTICS_H

#include <algorithm>
#include <chrono>

namespace stackelberg {
    class StackelbergStatistics {
        std::chrono::high_resolution_clock::time_point time_begin_stackelberg_search;

        std::chrono::milliseconds time_optimal_solver;
        std::chrono::milliseconds time_cost_bounded_solver;
        std::chrono::milliseconds time_solvers;
                        
        int num_follower_searches = 0;
        int num_follower_searches_optimal_solver = 0;
        int num_follower_searches_cost_bounded_solver = 0;
        int num_follower_searches_optimal_solution = 0;

        
        /* long follower_search_duration_sum = 0; */
        /* long reset_and_initialize_duration_sum = 0; */
        /* long leader_search_initialize_duration = 0; */
        /* int all_follower_states = 0; */
        /* int num_leader_op_paths = 0; */


    public:

        StackelbergStatistics();
        
        void dump() const;

        
        void stackelberg_search_initialized(std::chrono::high_resolution_clock::time_point t1);
        void stackelberg_search_finished();

        void follower_search_finished (std::chrono::milliseconds runtime,
                                       bool optimal_solver,
                                       bool optimal_solution );
    
    };

}

#endif
