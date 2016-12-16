/*
 * fixactionssearch.cpp
 *
 *  Created on: 12.12.2016
 *      Author: Patrick
 */

#include "fixactions_search.h"
#include <vector>
#include <math.h>
#include <unordered_set>
#include "option_parser.h"
#include "plugin.h"
#include <cassert>

using namespace std;

//vector<GlobalOperator> all_operators;
vector<GlobalOperator> fix_operators;
vector<GlobalOperator> attack_operators;
vector<GlobalOperator> attack_operators_with_fix_vars_preconds;

SuccessorGeneratorSwitch *fix_operators_successor_generator;
SuccessorGeneratorSwitch *attack_operators_for_fix_vars_successor_generator;

unordered_set<int> attack_vars;
int num_vars;
int num_attack_vars;
int num_fix_vars;
//vector<int> attack_vars_indices;

// Vector indexed by old id, encloses new id
vector<int> map_var_id_to_new_var_id;
vector<int> fix_variable_domain;
vector<string> fix_variable_name;
vector<vector<string> > fix_fact_names;
vector<int> fix_initial_state_data;
StateRegistry *fix_vars_state_registry;

vector<vector<bool>> commutative_fix_ops;

FixActionsSearch::FixActionsSearch(const Options &opts) :
		SearchEngine(opts) {
	// TODO Auto-generated constructor stub

}

FixActionsSearch::~FixActionsSearch() {
	// TODO Auto-generated destructor stub
}

void FixActionsSearch::initialize() {

	// Sort the operators
	for (size_t op_no = 0; op_no < g_operators.size(); op_no++) {
		string op_name = g_operators[op_no].get_name();
		cout << "For op: " << op_name << endl;

		size_t whitespace = op_name.find(" ");
		if (whitespace == string::npos) {
			whitespace = op_name.size();
		}
		string everything_before_whitespace = op_name.substr(0, whitespace);
		size_t underscore = everything_before_whitespace.find_last_of("_");
		if (underscore == string::npos) {
			cout << "No cost suffix found! Error in PDDL file?" << endl;
			exit(EXIT_INPUT_ERROR);
		}

		if (op_name.find("attack") == 0) {
			attack_operators.push_back(g_operators[op_no]);
			//all_operators.push_back(g_operators[op_no]);

			string prob = everything_before_whitespace.substr(underscore + 1);
			int success_prob_cost = parse_success_prob_cost(prob);
			cout << "success_prob_cost: " << success_prob_cost << endl;

		} else if (op_name.find("fix") == 0) {
			fix_operators.push_back(g_operators[op_no]);
			//all_operators.push_back(g_operators[op_no]);

			string invention_cost_string = everything_before_whitespace.substr(underscore + 1);
			int invention_cost = stoi(invention_cost_string);
			cout << "parsed invention_cost: " << invention_cost << endl;
		} else {
			cout << "No op prefix found! Error in PDDL file?" << endl;
			exit(EXIT_INPUT_ERROR);
		}
	}

	cout << "1" << endl;
	// Sort the variables
	divideVariables();

	cout << "2" << endl;

	clean_attack_actions();

	cout << "3" << endl;

	g_operators.clear();

	cout << "4" << endl;

	create_new_variable_indices();

	cout << "5" << endl;

	fix_operators_successor_generator = create_fix_vars_successor_generator(fix_operators, fix_operators);

	cout << "6" << endl;

	attack_operators_for_fix_vars_successor_generator = create_fix_vars_successor_generator(
			attack_operators_with_fix_vars_preconds, attack_operators);

	cout << "7" << endl;

	compute_commutative_fix_ops_matrix();

	cout << "8" << endl;

}

int FixActionsSearch::parse_success_prob_cost(string prob) {
	size_t backslash = prob.find("/");
	if (backslash == string::npos) {
		cout << "No correct success probability suffix found! Error in PDDL file?" << endl;
		exit(EXIT_INPUT_ERROR);
	}

	string numerator_string = prob.substr(0, backslash);
	string denominator_string = prob.substr(backslash + 1);
	double numerator = (double) stoi(numerator_string);
	double denominator = (double) stoi(denominator_string);
	return (int) (fabs(log2(numerator / denominator)) * 1000);
}

void FixActionsSearch::divideVariables() {
	for (size_t op_no = 0; op_no < attack_operators.size(); op_no++) {
		const vector<GlobalEffect> &effects = attack_operators[op_no].get_effects();
		for (size_t i = 0; i < effects.size(); i++) {
			int var = effects[i].var;
			attack_vars.insert(var);
		}
	}
	num_vars = g_variable_domain.size();
	num_attack_vars = attack_vars.size();
	num_fix_vars = num_vars - num_attack_vars;
	// copy(attack_vars.begin(), attack_vars.end(), back_inserter(attack_vars_indices));
	// sort(attack_vars_indices.begin(), attack_vars_indices.end());
}

void FixActionsSearch::clean_attack_actions() {

	for (size_t op_no = 0; op_no < attack_operators.size(); op_no++) {
		const GlobalOperator &op = attack_operators[op_no];
		const vector<GlobalCondition> &conditions = op.get_preconditions();
		std::vector<GlobalCondition> fix_preconditions;
		std::vector<GlobalCondition> attack_preconditions;
		for (size_t i = 0; i < conditions.size(); i++) {
			int var = conditions[i].var;
			if (attack_vars.find(var) != attack_vars.end()) {
				// This is a precondition on an attack var
				attack_preconditions.push_back(conditions[i]);
			} else {
				// This is a precondition on a fix var
				fix_preconditions.push_back(conditions[i]);
			}
		}

		GlobalOperator op_with_attack_preconds(op.is_axiom(), attack_preconditions, op.get_effects(), op.get_name(),
				op.get_cost(), op.get_cost2());
		attack_operators[op_no] = op_with_attack_preconds;

		GlobalOperator op_with_fix_preconds(op.is_axiom(), fix_preconditions, op.get_effects(), op.get_name(),
				op.get_cost(), op.get_cost2());
		attack_operators_with_fix_vars_preconds.push_back(op_with_fix_preconds);
	}
}

void FixActionsSearch::create_new_variable_indices() {
	int curr_attack_var_index = 0;
	int curr_fix_var_index = 0;

	map_var_id_to_new_var_id.resize(num_vars);

	for (int var = 0; var < num_vars; var++) {
		if (attack_vars.find(var) != attack_vars.end()) {
			// This is an attack var
			map_var_id_to_new_var_id[var] = curr_attack_var_index;
			curr_attack_var_index++;
		} else {
			// This is a fix var
			map_var_id_to_new_var_id[var] = curr_fix_var_index;
			curr_fix_var_index++;
		}
	}

	adjust_var_indices_of_ops(fix_operators);

	adjust_var_indices_of_ops(attack_operators);

	adjust_var_indices_of_ops(attack_operators_with_fix_vars_preconds);

	// Save the fix var stuff locally and clean g_variable_domain, g_variable_name and g_fact_names
	int num_vars_temp = num_vars;
	int var = 0;
	for (int i = 0; i < num_vars_temp; i++) {

		cout << "i: " << i << ", var: " << var << ", num_vars: " << num_vars << endl;

		if (attack_vars.find(var) == attack_vars.end()) {
			// This is a fix var

			cout << "is a fix var" << endl;

			// Save it to local vectors
			fix_variable_domain.push_back(g_variable_domain[i]);
			fix_variable_name.push_back(g_variable_name[i]);
			fix_fact_names.push_back(g_fact_names[i]);
			fix_initial_state_data.push_back(g_initial_state_data[i]);

			// Erase it from vectors
			g_variable_domain.erase(g_variable_domain.begin() + i);
			g_variable_name.erase(g_variable_name.begin() + i);
			g_fact_names.erase(g_fact_names.begin() + i);
			g_initial_state_data.erase(g_initial_state_data.begin() + i);

			// Decrement i and num_vars_temp, because an element was deleted.
			i--;
			num_vars_temp--;
		}
		// Increment var which is the absolute old var id
		var++;
	}

	// Changing indices in g_goal to attack_var indices and ensuring that there is no fix goal variable
	for (size_t i = 0; i < g_goal.size(); i++) {
		int var = g_goal[i].first;
		if (attack_vars.find(var) == attack_vars.end()) {
			cout << "There should be no goal defined for a non-attack var! Error in PDDL!" << endl;
			exit(EXIT_INPUT_ERROR);
		}
		g_goal[i].first = map_var_id_to_new_var_id[g_goal[i].first];
	}

	// Creating two new state_registries, one locally only for fix variables and one globally only for attack variables
	assert(fix_variable_domain.size() == fix_initial_state_data.size());
#ifndef NDEBUG
	for (unsigned var = 0; var < fix_variable_domain.size(); var++) {
		assert(fix_initial_state_data[var] < fix_variable_domain[var]);
	}
#endif
	IntPacker *fix_vars_state_packer = new IntPacker(fix_variable_domain);
	fix_vars_state_registry = new StateRegistry(fix_vars_state_packer, fix_initial_state_data);
	delete g_state_packer;
	g_state_packer = new IntPacker(g_variable_domain);
	delete g_state_registry;
	g_state_registry = new StateRegistry(g_state_packer, g_initial_state_data);
}

bool cond_comp_func(GlobalCondition cond1, GlobalCondition cond2) {
	return cond1.var < cond2.var;
}
bool eff_comp_func(GlobalEffect eff1, GlobalEffect eff2) {
	return eff1.var < eff2.var;
}
void FixActionsSearch::adjust_var_indices_of_ops(vector<GlobalOperator> &ops) {
	// Adjust indices in preconditions and effects of all operators in ops vector
	for (size_t op_no = 0; op_no < ops.size(); op_no++) {
		vector<GlobalCondition> &conditions = ops[op_no].get_preconditions();
		for (size_t cond_no = 0; cond_no < conditions.size(); cond_no++) {
			conditions[cond_no].var = map_var_id_to_new_var_id[conditions[cond_no].var];
		}

		vector<GlobalEffect> &effects = ops[op_no].get_effects();
		for (size_t eff_no = 0; eff_no < effects.size(); eff_no++) {
			effects[eff_no].var = map_var_id_to_new_var_id[effects[eff_no].var];
		}

		// Sort the conditions and effects by their respective var id
		sort(conditions.begin(), conditions.end(), cond_comp_func);
		sort(effects.begin(), effects.end(), eff_comp_func);
	}
}

/**
 * returns a SuccessorGeneratorSwitch based on the preconditions of the ops in pre_cond_ops and entailing the ops from ops vector in the leaves
 */
SuccessorGeneratorSwitch* FixActionsSearch::create_fix_vars_successor_generator(
		const vector<GlobalOperator> &pre_cond_ops, const vector<GlobalOperator> &ops) {
	int root_var_index = 0;

	cout << "root var is " << root_var_index << endl;

	SuccessorGeneratorSwitch *current_node = new SuccessorGeneratorSwitch(root_var_index,
			fix_variable_domain[root_var_index]);
	SuccessorGeneratorSwitch *root_node = current_node;

	for (size_t op_no = 0; op_no < pre_cond_ops.size(); op_no++) {
		cout << "Consider op " << op_no << endl;
		pre_cond_ops[op_no].dump(fix_variable_name, g_variable_name);
		vector<GlobalCondition> conditions = pre_cond_ops[op_no].get_preconditions();

		for (size_t cond_no = 0; cond_no < conditions.size(); cond_no++) {
			int var = conditions[cond_no].var;
			int val = conditions[cond_no].val;
			cout << "Consider precond with var: " << var << ", val: " << val << endl;

			while (var != current_node->switch_var) {
				if (current_node->default_generator == NULL) {
					int next_fix_var_index = current_node->switch_var + 1;
					current_node->default_generator = new SuccessorGeneratorSwitch(next_fix_var_index,
							fix_variable_domain[next_fix_var_index]);
				}

				current_node = (SuccessorGeneratorSwitch*) current_node->default_generator;
			}

			// Here: var == current_node->switch_var

			int next_fix_var_index = current_node->switch_var + 1;
			if (next_fix_var_index >= num_fix_vars) {
				if (current_node->generator_for_value[val] == NULL) {
					current_node->generator_for_value[val] = new SuccessorGeneratorGenerate();
				}
				((SuccessorGeneratorGenerate*) current_node->generator_for_value[val])->add_op(&ops[op_no]);

			} else {
				if (current_node->generator_for_value[val] == NULL) {
					current_node->generator_for_value[val] = new SuccessorGeneratorSwitch(next_fix_var_index,
							fix_variable_domain[next_fix_var_index]);
				}

				current_node = (SuccessorGeneratorSwitch*) current_node->generator_for_value[val];
				if (cond_no == (conditions.size() - 1)) {
					// This was the last cond.
					if (current_node->immediate_ops == NULL) {
						current_node->immediate_ops = new SuccessorGeneratorGenerate();
					}
					((SuccessorGeneratorGenerate*) current_node->immediate_ops)->add_op(&ops[op_no]);
				}
			}
		}

		// After processing an op, start at root again
		current_node = root_node;
	}

	return root_node;
}

/**
 * For every pair of ops op1 and op2, this method checks whether op1 has a precond on var v on which op2 has an effect and vice versa.
 * If this is the case, op1 and op2 are not commutative. If op1 and op2 both effect a var v with different effect values,
 * there are also not commutative. Otherwise, they are commutative.
 */
void FixActionsSearch::compute_commutative_fix_ops_matrix() {
	cout << "Begin compute_commutative_fix_ops_matrix()..." << endl;
	vector<bool> val(fix_operators.size());
	commutative_fix_ops.assign(fix_operators.size(), val);
	for (size_t op_no1 = 0; op_no1 < fix_operators.size(); op_no1++) {
		for (size_t op_no2 = op_no1 + 1; op_no2 < fix_operators.size(); op_no2++) {
			cout << "Comparing op1 with id " << op_no1 << ":" << endl;
			fix_operators[op_no1].dump(fix_variable_name);
			cout << "to op2 with id " << op_no2 << ":" << endl;
			fix_operators[op_no2].dump(fix_variable_name);

			const vector<GlobalCondition> &conditions1 = fix_operators[op_no1].get_preconditions();
			const vector<GlobalCondition> &conditions2 = fix_operators[op_no2].get_preconditions();
			const vector<GlobalEffect> &effects1 = fix_operators[op_no1].get_effects();
			const vector<GlobalEffect> &effects2 = fix_operators[op_no2].get_effects();

			bool commutative = true;
			int i_cond1 = 0, i_cond2 = 0, i_eff1 = 0, i_eff2 = 0;
			for (int var = 0; var < num_fix_vars; var++) {
				while (conditions1[i_cond1].var < var && i_cond1 < ((int) conditions1.size() - 1)) {
					i_cond1++;
				}
				while (conditions2[i_cond2].var < var && i_cond2 < ((int) conditions2.size() - 1)) {
					i_cond2++;
				}
				while (effects1[i_eff1].var < var && i_eff1 < ((int) effects1.size() - 1)) {
					i_eff1++;
				}
				while (effects2[i_eff2].var < var && i_eff2 < ((int) effects2.size() - 1)) {
					i_eff2++;
				}
				if ((conditions1[i_cond1].var == var && effects2[i_eff2].var == var)
						|| (conditions2[i_cond2].var == var && effects1[i_eff1].var == var)) {
					commutative = false;
				} else {
					if (effects1[i_eff1].var == var && effects2[i_eff2].var == var) {
						if (effects1[i_eff1].val != effects2[i_eff2].val) {
							commutative = false;
						}
					}
				}
				if (!commutative) {
					break;
				}
			}
			cout << "ops are commutative?: " << commutative << endl;
			commutative_fix_ops[op_no1][op_no2] = commutative;
			commutative_fix_ops[op_no2][op_no1] = commutative;
		}
	}
}

int num_recursive_calls = 0;
void expand_all_successors(const GlobalState &state, vector<const GlobalOperator*> &op_sequence, vector<int> &sleep,
		bool use_partial_order_reduction) {
	num_recursive_calls++;

	vector<const GlobalOperator *> all_attack_operators;
	attack_operators_for_fix_vars_successor_generator->generate_applicable_ops(state, all_attack_operators);
	g_operators.clear();
	for(size_t op_no = 0; op_no < all_attack_operators.size(); op_no++) {
		g_operators.push_back(*all_attack_operators[op_no]);
	}
	cout << "New g_operators size is: " << g_operators.size() << endl;


	cout << "expand all successors of state: " << endl;
	state.dump_fdr(fix_variable_domain, fix_variable_name);
	vector<const GlobalOperator *> all_operators;
	cout << "8" << endl;
	fix_operators_successor_generator->generate_applicable_ops(state, all_operators);

	cout << "9" << endl;

	for (size_t op_no = 0; op_no < all_operators.size(); op_no++) {
		cout << "10" << endl;
		if (find(op_sequence.begin(), op_sequence.end(), all_operators[op_no]) != op_sequence.end()) {
			// Continue, if op is already in sequence
			cout << "11 op already in sequence" << endl;
			continue;
		}
		if (sleep[op_no] != 0) {
			// Continue, if op is in sleep set
			cout << "12 op is skipped, because it's in the sleep set" << endl;
			continue;
		}
		cout << "13" << endl;

		// Add all ops before op_no in all_operators to sleep set if they are commutative
		if (use_partial_order_reduction) {
			for (size_t op_no2 = 0; op_no2 < op_no; op_no2++) {
				if (commutative_fix_ops[op_no][op_no2]) {
					sleep[op_no2]++;
				}
			}
		}
		op_sequence.push_back(all_operators[op_no]);
		const GlobalState &next_state = fix_vars_state_registry->get_successor_state(state, *all_operators[op_no]);
		expand_all_successors(next_state, op_sequence, sleep, use_partial_order_reduction);
		cout << "14" << endl;

		// Remove all ops before op_no in all_operators from sleep set if they are commutative
		if (use_partial_order_reduction) {
			for (size_t op_no2 = 0; op_no2 < op_no; op_no2++) {
				if (commutative_fix_ops[op_no][op_no2]) {
					sleep[op_no2]--;
				}
			}
		}

		op_sequence.pop_back();
	}
}

SearchStatus FixActionsSearch::step() {
	vector<const GlobalOperator *> op_sequnce;
	vector<int> sleep(fix_operators.size(), 0);
	expand_all_successors(fix_vars_state_registry->get_initial_state(), op_sequnce, sleep, true);
	cout << "They were " << num_recursive_calls << " calls to expand_all_successors." << endl;
	cout << "15" << endl;
	exit(EXIT_CRITICAL_ERROR);
	return IN_PROGRESS;
}

void FixActionsSearch::add_options_to_parser(OptionParser &parser) {
	SearchEngine::add_options_to_parser(parser);
}

SearchEngine * _parse(OptionParser & parser) {
	FixActionsSearch::add_options_to_parser(parser);
	Options opts = parser.parse();
	if (!parser.dry_run()) {
		return new FixActionsSearch(opts);
	}
	return NULL;
}

Plugin<SearchEngine> _plugin("fixsearch", _parse);
