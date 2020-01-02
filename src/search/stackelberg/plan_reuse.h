
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
