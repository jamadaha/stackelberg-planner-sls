#ifndef STACKELBERG_STATISTICS_H
#define STACKELBERG_STATISTICS_H

#include <algorithm>
#include <chrono>

namespace stackelberg {

    class FollowerSolution;
    class StackelbergStatistics {

        const bool print_follower_search_info;
        std::chrono::high_resolution_clock::time_point time_begin_stackelberg_search;

        std::chrono::milliseconds time_optimal_solver;
        std::chrono::milliseconds time_cost_bounded_solver;
        std::chrono::milliseconds time_solvers;
                        
        int num_follower_searches = 0;
        int num_follower_searches_optimal_solver = 0;
        int num_follower_searches_cost_bounded_solver = 0;
        int num_follower_searches_optimal_solution = 0;
        int num_follower_searches_solved = 0;

    public:
        StackelbergStatistics(bool print_follower_search_info);
        
        void dump() const;

        void stackelberg_search_initialized(std::chrono::high_resolution_clock::time_point t1);
        void stackelberg_search_finished();

        void follower_search_finished (std::chrono::milliseconds runtime,
                                       bool optimal_solver,
                                       const FollowerSolution & solution);
    
    };

}

#endif
