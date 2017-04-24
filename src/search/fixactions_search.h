/*
 * fixactionssearch.h
 *
 *  Created on: 12.12.2016
 *      Author: Patrick
 */

#ifndef SRC_SEARCH_FIXACTIONS_SEARCH_H_
#define SRC_SEARCH_FIXACTIONS_SEARCH_H_

#include <unordered_set>
#include <limits>
#include "search_engine.h"
#include "successor_generator.h"
#include "attack_success_prob_reuse_heuristic.h"
#include "per_fix_state_information.h"
#include "sort_fixactions_by_attacker_reward.h"
#include "delrax_search.h"

template<typename T1, typename T2, typename T3> using triple = std::tuple<T1, T2, T3>;

#define ATTACKER_TASK_UNSOLVABLE 0 // numeric_limits<int>::max()
#define NO_ATTACKER_COST numeric_limits<int>::min() // 0

struct FixSearchInfoAttackPlan {
        int attack_plan_prob_cost;
        std::vector<int> attack_plan;
        FixSearchInfoAttackPlan(int _attack_plan_prob_cost = -1, std::vector<int> _attack_plan = std::vector<int>())
        : attack_plan_prob_cost(_attack_plan_prob_cost),
		  attack_plan(_attack_plan){ }
};

struct FixSearchInfoFixSequence {
        int fix_actions_cost;
        bool already_in_frontier;
        FixSearchInfoFixSequence(int _fix_actions_cost = -1, bool _already_in_frontier = false)
        : fix_actions_cost(_fix_actions_cost),
		  already_in_frontier(_already_in_frontier){ }
};



class FixActionsSearch: public SearchEngine
{
private:
	bool use_partial_order_reduction = true;
	bool check_parent_attack_plan_applicable = true;
	bool check_fix_state_already_known = true;
	bool do_attack_op_dom_pruning = true;
	bool sort_fix_ops_stupid = false;
	bool sort_fix_ops_advanced = true;

	std::vector<GlobalOperator> fix_operators;
	std::vector<GlobalOperator> attack_operators;
	std::vector<GlobalOperator> attack_operators_with_fix_vars_preconds;

	SuccessorGeneratorSwitch *fix_operators_successor_generator = NULL;
	SuccessorGeneratorSwitch *attack_operators_for_fix_vars_successor_generator = NULL;

	std::vector<bool> attack_vars;
	int num_vars = 0;
	int num_attack_vars = 0;
	int num_fix_vars = 0;

	std::vector<int> map_var_id_to_new_var_id; // Vector indexed by old id, encloses new id
	std::vector<int> fix_variable_domain;
	std::vector<std::string> fix_variable_name;
	std::vector<std::vector<std::string> > fix_fact_names;
	std::vector<int> fix_vars_attacker_preconditioned;

	std::vector<int> fix_initial_state_data;
	StateRegistry *fix_vars_state_registry = NULL;

	std::vector<std::vector<bool>> commutative_fix_ops;
	std::vector<std::vector<bool>> dependent_fix_ops;
	std::vector<std::vector<std::vector<const GlobalOperator *>>> deleting_fix_facts_ops;
	std::vector<std::vector<std::vector<const GlobalOperator *>>> achieving_fix_facts_ops;

	std::vector<std::vector<int>> dominated_attack_op_ids;

	SearchEngine* search_engine;
	Heuristic* attack_heuristic;

	std::vector<triple<int, int, std::vector<std::vector<const GlobalOperator* >>>> pareto_frontier;
	int fix_action_costs_for_no_attacker_solution = std::numeric_limits<int>::max();
	PerFixStateInformation<FixSearchInfoAttackPlan> fix_search_node_infos_attack_plan;
	PerStateInformation<FixSearchInfoFixSequence> fix_search_node_infos_fix_sequence;
	int initial_fix_actions_budget = UNLTD_BUDGET;
	double attack_budget_factor;
	double fix_budget_factor;

	SortFixActionsByAttackerReward *sortFixActionsByAttackerReward = NULL;

	int num_recursive_calls = 0;
	int num_attacker_searches = 0;
	long attack_search_duration_sum = 0;
	long reset_and_initialize_duration_sum = 0;
	long fix_search_initialize_duration = 0;
	int all_attacker_states = 0;
	int spared_attacker_searches_because_fix_state_already_seen = 0;
	int spared_attacker_searches_because_parent_plan_applicable = 0;
	int num_fix_op_paths = 0;
	int num_recursive_calls_for_sorting = 0;

	void iterate_applicable_ops(const std::vector<const GlobalOperator*>& applicable_ops_after_pruning, const GlobalState& state, const std::vector<int> &attack_plan, int attack_plan_cost, std::vector<const GlobalOperator*>& fix_ops_sequence, std::vector<int>& sleep, AttackSearchSpace* attack_heuristic_search_space, bool recurse, std::vector<int> &recursive_attacker_costs);

protected:
    virtual void initialize();
    virtual SearchStatus step();
    void sort_operators();
    int parse_success_prob_cost(std::string prob);
    double prob_cost_to_prob(int prob_cost);
    void divide_variables();
    void clean_attack_actions();
    void create_new_variable_indices();
    void adjust_var_indices_of_ops(std::vector<GlobalOperator> &ops);
    void check_fix_vars_attacker_preconditioned();
    SuccessorGeneratorSwitch* create_successor_generator(const std::vector<int> &variable_domain, const std::vector<GlobalOperator> &pre_cond_ops, const std::vector<GlobalOperator> &ops);
    void compute_commutative_and_dependent_fix_ops_matrices();
    void compute_op_dominance_relation(const std::vector<GlobalOperator> &ops, std::vector<std::vector<int>> &dominated_op_ids);
    void compute_fix_facts_ops_sets();
    void get_all_dependent_ops(const GlobalOperator *op, std::vector<const GlobalOperator *> &result);
    void prune_applicable_fix_ops_sss (const GlobalState &state, const std::vector<int> &attack_plan, const std::vector<const GlobalOperator *> &applicable_ops, std::vector<const GlobalOperator *> &result);
    void prune_dominated_ops(std::vector<const GlobalOperator*> &ops, std::vector<std::vector<int>> dominated_op_ids);
    std::string fix_state_to_string(const GlobalState &state);
    std::string ops_to_string(std::vector<const GlobalOperator *> &ops);
    int compute_pareto_frontier(const GlobalState &state, std::vector<const GlobalOperator*> &fix_ops_sequence, int fix_actions_cost, const std::vector<int> &parent_attack_plan, int parent_attack_plan_cost, std::vector<int> &sleep, bool recurse);
    void add_node_to_pareto_frontier(triple<int, int, std::vector<std::vector<const GlobalOperator*>>> &node);

    void dump_op_sequence(const std::vector<const GlobalOperator*> &op_sequence, std::ostringstream &json);
    void dump_op_sequence_sequence(const std::vector<std::vector<const GlobalOperator*>> &op_sequence_sequence, std::ostringstream &json);
    void dump_pareto_frontier_node(triple<int, int, std::vector<std::vector<const GlobalOperator*>>> &node, std::ostringstream &json);
    void dump_pareto_frontier ();

public:
    FixActionsSearch(const Options &opts);
    virtual ~FixActionsSearch();
    static void add_options_to_parser(OptionParser &parser);
};

#endif /* SRC_SEARCH_FIXACTIONS_SEARCH_H_ */
