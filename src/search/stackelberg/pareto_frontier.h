#ifndef PARETO_FRONTIER_H
#define PARETO_FRONTIER_H

#include <vector>
#include <sstream>
#include <iostream>
#include <limits>

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
        int follower_cost_upper_bound;        
        int leader_action_costs_for_follower_upper_bound;
    
    public:
        ParetoFrontier() : complete_up_to(-1),
            follower_cost_upper_bound(std::numeric_limits<int>::max()),
            leader_action_costs_for_follower_upper_bound(std::numeric_limits<int>::max())
            {
            }

        int get_leader_action_costs_for_follower_upper_bound () const {
            return leader_action_costs_for_follower_upper_bound;
        }
        
        void add_node(int leader_cost, int follower_cost,
                      const std::vector<const GlobalOperator *> & leader_plan,
                      const std::vector<int> & follower_plan);

        void add_node(int leader_cost, int follower_cost,
                      const std::vector<const GlobalOperator *> & leader_plan,
                      const std::vector<const GlobalOperator *> & follower_plan); 


        void dump(const StackelbergTask & task);

        void set_follower_cost_upper_bound(int upper_bound) {
            if (upper_bound < follower_cost_upper_bound) {
                follower_cost_upper_bound = upper_bound;
                std::cout << "Follower cost upper bound: " << upper_bound << std::endl;
            }
        }

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
