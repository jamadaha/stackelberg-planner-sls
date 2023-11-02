#ifndef STACKELBERG_FOLLOWER_TASK_H
#define STACKELBERG_FOLLOWER_TASK_H

#include <limits>
#include <vector>
#include <cassert>

#include "../state_id.h"

class GlobalOperator;

namespace stackelberg {

    class FollowerTask { 
    public:
        int upper_bound; //upper bound provided by the following plan 
        std::vector<int> follower_plan;
        int lower_bound;

        StateID leader_state_id;
        int leader_cost;
        
        bool has_optimal_solution() const {
            assert (lower_bound <= upper_bound);
            return lower_bound == upper_bound;
        }
        
        FollowerTask () : upper_bound(std::numeric_limits<int>::max()),
            lower_bound (0), leader_state_id (StateID::no_state),
            leader_cost (std::numeric_limits<int>::max()) {
        }

        std::vector<const GlobalOperator *> get_leader_plan() const;

    };
    
    
}



#endif
