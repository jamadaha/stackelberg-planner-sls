/*
 * stackelberg_search is a copy of fix action search to introduce modifications in the way
 * that we call the search engine so that it is compatible with symbolic search engines.
 * 
 * I'm also removing optional stuff that is currently unnecessary here to simplify the
 * code and understand it better. We can re-add those options later on.
 */
#ifndef STACKELBERG_TASK_H
#define STACKELBERG_TASK_H

#include <vector>

namespace stackelberg {

    class StackelbergTask {
        std::vector<GlobalOperator> leader_operators;
        std::vector<GlobalOperator> follower_operators;
        std::vector<GlobalOperator> follower_operators_with_all_preconds;
        std::vector<GlobalOperator> follower_operators_with_leader_vars_preconds;

        std::vector<bool> follower_vars;
        std::vector<int> follower_vars_indizes;

        std::vector<bool> leader_vars;
        std::vector<int> leader_vars_indizes;

        int num_vars;
        int num_follower_vars;
        int num_leader_vars;

        std::vector<int> map_var_id_to_new_follower_var_id; // Vector indexed by old id, encloses new attack var id
        std::vector<int> map_var_id_to_new_leader_var_id; // Vector indexed by old id, encloses new attack var id
        std::vector<int> map_leader_var_id_to_orig_var_id; // Vector indexed by fix var id, encloses original id
        std::vector<int> map_follower_var_id_to_orig_var_id; // Vector indexed by attack var id, encloses original id

        std::vector<int> leader_variable_domain;
        std::vector<std::string> leader_variable_name;
        std::vector<std::vector<std::string> > leader_fact_names;
        std::vector<int> leader_vars_attacker_preconditioned;

        std::vector<int> leader_initial_state_data;


        void sort_operators();
        int parse_success_prob_cost(std::string prob);
        double prob_cost_to_prob(int prob_cost);
        void divide_variables();
        void clean_follower_actions();
        void create_new_variable_indices();
        void adjust_var_indices_of_ops(std::vector<GlobalOperator> &ops, const std::vector<int> &map_precond_var_id_to_new_var_id, const std::vector<int> &map_eff_var_id_to_new_var_id);
        void check_leader_vars_attacker_preconditioned();

    public:
        StackelbergTask();
        
        void dump_statistics() const ;

        const std::vector<int> & get_leader_vars_attacker_preconditioned() {
            return leader_vars_attacker_preconditioned;
        }

        const GlobalOperator & get_follower_operator(int i) const {
            return follower_operators[i];
        }
    };
}
#endif

