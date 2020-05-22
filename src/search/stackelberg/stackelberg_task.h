/*
 * stackelberg_task represents the leader+follower tasks. 
 */
#ifndef STACKELBERG_TASK_H
#define STACKELBERG_TASK_H

#include <vector>
#include <limits>
#include <memory>
  #include <set>


#include "../global_operator.h"

namespace stackelberg {

    class FollowerTask;

    class StackelbergTask {

        std::vector<int> global_operator_id_leader_ops;
        std::vector<int> global_operator_id_follower_ops;

                
        std::vector<GlobalOperator> leader_operators;
        std::vector<GlobalOperator> follower_operators;
        std::vector<GlobalOperator> follower_operators_with_all_preconds;
        std::vector<GlobalOperator> follower_operators_with_leader_vars_preconds;

        std::vector<bool> follower_vars;
        std::vector<int> follower_vars_indizes;

        std::vector<bool> leader_vars;
        std::vector<int> leader_vars_indizes;

        std::vector<bool> follower_precondition_vars;


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
        std::vector<int> leader_vars_follower_preconditioned;

        std::vector<int> leader_initial_state_data;

        std::vector<std::vector<std::vector<const GlobalOperator *>>> deleting_leader_facts_ops;
	std::vector<std::vector<std::vector<const GlobalOperator *>>> achieving_leader_facts_ops;
        
        /* std::unique_ptr<FollowerTask> minimal_follower_task; */
        
        void sort_operators();
        int parse_success_prob_cost(std::string prob);
        double prob_cost_to_prob(int prob_cost);
        void divide_variables();
        void clean_follower_actions();
        void create_new_variable_indices();
        void adjust_var_indices_of_ops(std::vector<GlobalOperator> &ops, const std::vector<int> &map_precond_var_id_to_new_var_id, const std::vector<int> &map_eff_var_id_to_new_var_id);
        void compute_leader_facts_ops_sets();

        void check_leader_vars_follower_preconditioned();

    public:
	static const int FOLLOWER_TASK_UNSOLVABLE = std::numeric_limits<int>::max();

        StackelbergTask();

        bool is_stackelberg_task() const {
            return !leader_operators.empty();
        }
        
        void dump_statistics() const ;


	const std::vector<GlobalOperator> & get_leader_operators() const {
	    return leader_operators;
	}

	const std::vector<GlobalOperator> & get_follower_operators_with_all_preconds() const {
	    return follower_operators_with_all_preconds;    
	}

	const std::vector<GlobalOperator> & get_follower_operators_with_leader_vars_preconds () const {
	    return follower_operators_with_leader_vars_preconds;    
	}
	
        const std::vector<int> & get_leader_vars_follower_preconditioned() {
            return leader_vars_follower_preconditioned;
        }

        const GlobalOperator & get_follower_operator(int i) const {
            return follower_operators[i];
        }

        std::vector<int> get_follower_state(const GlobalState &leader_state) const;

	int max_leader_action_cost() const;

	std::string leader_state_to_string(const GlobalState &state);

	std::unique_ptr<StateRegistry> get_leader_state_registry() const;
	
	const std::vector<int> & get_leader_variable_domain() const {
	    return leader_variable_domain;
	}

	int get_num_leader_vars() const  {
	    return num_leader_vars;
	}


	const std::vector<int> &  get_map_leader_var_id_to_orig_var_id() const {
	    return map_leader_var_id_to_orig_var_id;
	}

	const std::vector<int> & get_follower_vars_indizes() const {
	    return follower_vars_indizes;
	}

        void compute_always_applicable_follower_ops(std::vector<GlobalOperator> &ops) const;

        int get_map_leader_var_id_to_orig_var_id(int leader_var_id) {
            return map_leader_var_id_to_orig_var_id[leader_var_id];
        }

        const std::vector<int> & get_global_operator_id_leader_ops () const {
            return global_operator_id_leader_ops;
        }

        const std::vector<int> & get_global_operator_id_follower_ops () const {
            return global_operator_id_follower_ops;
        }

        
        const std::vector<bool> & get_pattern_leader_vars () const {
            return leader_vars;
        }

        

        std::set<int> get_leader_only_vars () const;
        std::set<int> get_follower_only_vars () const;
        
        bool is_leader_only_var (int var) const;
        bool is_follower_only_var (int var) const;
        bool is_follower_effect_var (int var) const;
        bool is_follower_static_var (int v) const;

        const std::vector<bool> & get_follower_vars () const {
            return follower_vars;
        }



        friend class PartialOrderReduction;	
    };
}
#endif

