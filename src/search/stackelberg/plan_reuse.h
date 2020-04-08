
/* We keep a plan_registry, where each action sequence is mapped to a unique_id, which is
 * what is actually stored in each node. That way, we can easily keep for every node what
 * is the parent plan. Also, we can store a PerPlanInformation object where we register
 * any relevant information for the plan.*/

namespace stackelberg {

    class FollowerPlan {

    };
    
    class PlanReuse {
        std::vector<FollowerPlan> plans;

        void add_plan(FollowerPlan & plan); 

        //
        virtual int get_upper_bound (const FollowerTask &task, int desired_bound) = 0;
    };


    class SymbolicPlanReuse : public PlanReuse {

    };


}





    bool parent_follower_plan_applicable = false;
    if (check_parent_follower_plan_applicable && parent_follower_plan.size() > 0) {
        /*cout << "parent attack plan: " << endl;
          for (size_t op_no = 0; op_no < parent_follower_plan.size(); op_no++) {
          follower_operators_with_leader_vars_preconds[parent_follower_plan[op_no]].dump();
          }*/

        parent_follower_plan_applicable = true;
        // Check whether parent_follower_plan is still applicable in current fix-state
        for (size_t op_no = 0; op_no < parent_follower_plan.size(); op_no++) {
            if (!follower_operators_with_leader_vars_preconds[parent_follower_plan[op_no]].is_applicable(state)) {
                parent_follower_plan_applicable = false;
                break;
            }
        }

        //TODO: One should check if the previous plan still satisfies the goal!
    }
