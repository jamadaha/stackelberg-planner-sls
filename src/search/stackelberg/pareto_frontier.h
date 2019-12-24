#ifndef PARETO_FRONTIER_H
#define PARETO_FRONTIER_H

#include <vector>
#include <sstream>

class GlobalOperator;

namespace stackelberg {
    
    class StackelbergTask;
    class ParetoFrontier;
    class ParetoFrontierNode {
        int leader_cost;
        int follower_cost;
        std::vector<std::vector<const GlobalOperator* >> leader_plans;
        std::vector<int> follower_plan;

    public:

        ParetoFrontierNode (int leader_cost_, int follower_cost_,
                            const std::vector<const GlobalOperator* > &leader_plan_,
                            const std::vector<int> & follower_plan_) : leader_cost(leader_cost_),
            follower_cost(follower_cost_),  follower_plan(follower_plan_) {
            leader_plans.push_back (leader_plan_);
        
        }

        void add_leader_plan(const std::vector<const GlobalOperator* > &leader_plan) {
            leader_plans.push_back(leader_plan);
        }
        
    
        void dump(const StackelbergTask & task, std::ostringstream &json);

        friend class ParetoFrontier;
    };

    class ParetoFrontier {    
        std::vector<ParetoFrontierNode> frontier;

        int complete_up_to;
    
        void add_node_at_end(ParetoFrontierNode && node) ;
    
    public:
        void add_node(int leader_cost, int follower_cost,
                      const std::vector<const GlobalOperator *> & leader_plan,
                      const std::vector<int> & follower_plan);


        void dump(const StackelbergTask & task);

        void set_complete_up_to(int leader_cost) {
            complete_up_to = leader_cost;
        }
    
        bool is_complete(int follower_upper_bound) const {
            for (auto it = frontier.rbegin(); it != frontier.rend(); ++it) {
                if (it->leader_cost <= complete_up_to){
                    if (it->follower_cost == follower_upper_bound) {
                        return true;
                    }
                    break;
                }
            }
            return false;
        }
    
    };
}

#endif
