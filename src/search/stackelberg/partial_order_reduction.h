#ifndef STACKELBERG_PARTIAL_ORDER_REDUCTION_H
#define STACKELBERG_PARTIAL_ORDER_REDUCTION_H

#include <vector>

class GlobalState;
class GlobalOperator;

namespace stackelberg {

    class StackelbergTask;
    
    class PartialOrderReduction {

        const StackelbergTask * task;
        std::vector<std::vector<bool>> commutative_leader_ops;
	std::vector<std::vector<bool>> dependent_leader_ops;

        void compute_commutative_and_dependent_leader_ops_matrices(const StackelbergTask * task);
    public:
        PartialOrderReduction(const StackelbergTask * task);

        void prune_applicable_leader_ops_sss (const GlobalState &state, const std::vector<int> &follower_plan, const std::vector<const GlobalOperator *> &applicable_ops, std::vector<const GlobalOperator *> &result);

        void get_all_dependent_ops(const GlobalOperator *op, std::vector<const GlobalOperator *> &result) const;


        bool are_commutative_leader_ops(int op1_id, int op2_id) const {
            return commutative_leader_ops[op1_id][op2_id];
        }

        
    };

}

#endif
