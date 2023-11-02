/*
 * stackelberg_task represents the leader+follower tasks. 
 */
#ifndef DOMINANCE_STACKELBERG_TASK_H
#define DOMINANCE_STACKELBERG_TASK_H

#include <vector>
#include <set>
namespace stackelberg {

    class StackelbergTask;

    class DominanceStackelbergVariable {
        int var;

        std::vector<std::vector<bool>> rel;
        
    public:
        DominanceStackelbergVariable(int var, const std::set<int> & values_in_precondition);
                
        bool dominates(const std::vector<int> & t, const std::vector<int> & s) const;
    };

    class DominanceStackelbergTask {

        std::vector<DominanceStackelbergVariable> dominance;  
        
    public:
        DominanceStackelbergTask(const StackelbergTask & task);
        bool dominates(const std::vector<int> & t, const std::vector<int> & s) const;

    };

}


#endif
