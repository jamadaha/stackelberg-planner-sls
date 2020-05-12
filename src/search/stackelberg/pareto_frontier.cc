#include "pareto_frontier.h"

#include <cassert>
#include <iostream>
#include <fstream>

#include "stackelberg_task.h"
#include "../global_operator.h"

using namespace std;

namespace stackelberg {

    void ParetoFrontier::add_node(int leader_cost, int follower_cost,
                                  const vector<const GlobalOperator *> & leader_plan,
                                  const vector<int> & follower_plan) {


        if (follower_cost >= follower_cost_upper_bound) {
            leader_action_costs_for_follower_upper_bound = leader_cost;
        }
       
        if (frontier.empty()) {
            frontier.push_back(ParetoFrontierNode(leader_cost,  follower_cost, leader_plan, follower_plan));
            //cout << "New node in the pareto frontier: (" << leader_cost << "," << follower_cost << ")" << endl;

            return;
        }

        auto last_item = frontier.rbegin();
        if (leader_cost > last_item->leader_cost) {
            if( follower_cost > last_item->follower_cost) {
                frontier.push_back(ParetoFrontierNode(leader_cost, follower_cost, leader_plan, follower_plan));
                //cout << "New node in the pareto frontier: (" << leader_cost << "," << follower_cost << ")" << endl;
            }
            return;
        }

        ParetoFrontierNode node (leader_cost, follower_cost, leader_plan, follower_plan);
        auto it = lower_bound(frontier.begin(), frontier.end(), node,
                              [](const ParetoFrontierNode & n1, const ParetoFrontierNode & n2) {
                                  return n1.leader_cost < n2.leader_cost;
                              }
            );

        // First check whether follower_prob_costs == Intmax and fix_actions_cost <
        // fix_action_costs_for_attacker_upper_bound
        // if (follower_cost >= attacker_cost_upper_bound) {
        //     if (leader_cost < fix_action_costs_for_attacker_upper_bound) {
        //         fix_action_costs_for_attacker_upper_bound = leader_cost;
        //     }
        // }
    
    
        if (it != frontier.begin() && (it - 1)->follower_cost >= follower_cost) {
            // The new node is dominated by existing node with fewer fix action costs
            return;
        }

        if (it->leader_cost == leader_cost) {
            if (it->follower_cost < follower_cost) {
                it = frontier.erase(it);
                it = frontier.insert(it, node);
                //cout << "New node in the pareto frontier: (" << leader_cost << "," << follower_cost << ")" << endl;

#ifdef FIX_SEARCH_DEBUG
                cout << "added node with fix cost: " << leader_cost << " and attack cost: " <<
                    follower_cost << " to frontier" << endl;
#endif
                it++;
            } else if (it->follower_cost == follower_cost) {
                if (leader_plan.empty()) {
                    // Do not add the empty fix ops sequence again
                    // Assert that the the empty fix ops sequence is the only element her in the frontier
                    assert(it->leader_plans.size() == 1 && it->leader_plans[0].empty());
                    return;
                }
#ifdef FIX_SEARCH_DEBUG
                cout << "added additional fix action sequence to node with with fix cost: " <<
                    leader_cost << " and attack cost: " << follower_cost << endl;
#endif
                it->add_leader_plan(leader_plan);
                return;
            } else {
                return;
            }
        } else {
            it = frontier.insert(it, node);
            it++;
            //cout << "New node in the pareto frontier: (" << leader_cost << "," << follower_cost << ")" << endl;

#ifdef FIX_SEARCH_DEBUG
            cout << "added node with fix cost: " << leader_cost << " and attack cost: " <<
                follower_cost << " to frontier" << endl;
#endif
        }

        while (it != frontier.end() && it->follower_cost <= follower_cost) {
            it = frontier.erase(it);
        }

    }



    void dump_follower_op_sequence(const vector<int> &op_sequence, const StackelbergTask & task, std::ostringstream &json)
    {
	json << " [";
        if (op_sequence.size() < 1) {
            cout << "\t\t <unsolvable>" << endl;
            json << "]";
            return;
        }

        for (size_t i = 0; i < op_sequence.size(); ++i) {
            json << (i > 0 ? ", " : "") << "\"" << task.get_follower_operator(op_sequence[i]).get_name() << "\"";
            cout << "\t\t " << task.get_follower_operator(op_sequence[i]).get_name() << endl;
        }
        json << "]";
    }

    void dump_op_sequence(const vector<const GlobalOperator *>
                          &op_sequence, std::ostringstream &json)
    {

	json << "  [";
        if (op_sequence.size() < 1) {
            cout << "\t\t\t\t <empty sequence>" << endl;
            json << "]";
            return;
        }

        for (size_t i = 0; i < op_sequence.size(); ++i) {
            json << (i > 0 ? ", " : "") << "\"" << op_sequence[i]->get_name() << "\"";
            cout << "\t\t\t\t " << op_sequence[i]->get_name() << endl;
        }
        json << "]";
    }


    void ParetoFrontier::add_node(int leader_cost, int follower_cost,
                                  const std::vector<const GlobalOperator *> & leader_plan,
                                  const std::vector<const GlobalOperator *> & follower_plan) {
        vector<int> follower_plan_int;
            
        for (const  GlobalOperator * op : follower_plan){
            follower_plan_int.push_back(op->get_op_id());
        }
                
        add_node(leader_cost, follower_cost, leader_plan, follower_plan_int);
    }

    void dump_op_sequence_sequence(const
                                   vector<vector<const GlobalOperator *>> &op_sequence_sequence, std::ostringstream &json)
    {
        for (size_t i = 0; i < op_sequence_sequence.size(); ++i) {
            if (i > 0)  {
                json << ",\n";
            }
            cout << "\t\t\t sequence " << i << ":" << endl;
            dump_op_sequence(op_sequence_sequence[i], json);
        }
    }




    void ParetoFrontierNode::dump(const StackelbergTask & task, std::ostringstream &json)
    {
        cout << "\t fix ops costs: " << leader_cost << ", attacker cost: " <<
            follower_cost << ": " << endl;
        cout << "\t fix action sequences: " << endl;
        json << "{"
             << "\"attacker cost\": " << abs(follower_cost)
             << ", \"defender cost\": " << leader_cost
             << ", \"sequences\": [";
        dump_op_sequence_sequence(leader_plans, json);
        json << "]";
        cout << "\t attacker plan: " << endl;
        json << ", \"attacker plan\": ";
        dump_follower_op_sequence(follower_plan, task, json);
        json << "}";
    }

    void ParetoFrontier::dump(const StackelbergTask & task)
    {
        std::ostringstream json;
        json << "[";
        cout << "Pareto-frontier size: " << frontier.size() << endl;
        cout << "Pareto-frontier: ";
        for (const auto  &  node : frontier) {
            cout << "(" << node.leader_cost << "," << node.follower_cost << ") ";
        }                
        
        cout << endl;
        
        for (size_t i = 0; i < frontier.size(); ++i) {
            if(i > 0) {
    		json << ",\n";
            }
            frontier[i].dump(task, json);
        }
        cout << "END Pareto-frontier" << endl;

        json << "]";
        std::ofstream out;
        out.open("pareto_frontier.json");
        out << json.str();
        out.close();
    }

}

