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
#include "successor_generator.h"

using namespace std;

vector<GlobalOperator> all_operators;
vector<GlobalOperator> fix_operators;
vector<GlobalOperator> attack_operators;
SuccessorGeneratorSwitch *fix_actions_successor_generator;

unordered_set<int> attack_vars;
vector<int> attack_vars_indices;

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
			all_operators.push_back(g_operators[op_no]);

			string prob = everything_before_whitespace.substr(underscore + 1);
			int success_prob_cost = parse_success_prob_cost(prob);
			cout << "success_prob_cost: " << success_prob_cost << endl;

		} else if (op_name.find("fix") == 0) {
			fix_operators.push_back(g_operators[op_no]);
			all_operators.push_back(g_operators[op_no]);

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

	create_fix_actions_successor_generator();

	cout << "5" << endl;

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
	for (size_t op_no = 0; op_no < g_operators.size(); op_no++) {
		const vector<GlobalEffect> &effects = g_operators[op_no].get_effects();
		for (size_t i = 0; i < effects.size(); i++) {
			int var = effects[i].var;
			attack_vars.insert(var);
		}
	}
	copy(attack_vars.begin(), attack_vars.end(), back_inserter(attack_vars_indices));
	sort(attack_vars_indices.begin(), attack_vars_indices.end());
}

void FixActionsSearch::clean_attack_actions() {

	for (size_t op_no = 0; op_no < attack_operators.size(); op_no++) {
		const GlobalOperator &op = g_operators[op_no];
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
		// TODO we need to do sth. with the fix_preconditions
		GlobalOperator new_op(op.is_axiom(), attack_preconditions, op.get_effects(), op.get_name(), op.get_cost(),
				op.get_cost2());
		attack_operators[op_no] = new_op;
	}
}

void FixActionsSearch::create_fix_actions_successor_generator() {
	int root_var_index = get_next_fix_var(-1);
	if (root_var_index == -1) {
		// Nothing to do here
		return;
	}

	cout << "root var is " << root_var_index << endl;

	SuccessorGeneratorSwitch *current_node = new SuccessorGeneratorSwitch(root_var_index);
	fix_actions_successor_generator = current_node;
	for (size_t op_no = 0; op_no < fix_operators.size(); op_no++) {
		cout << "Consider op " << op_no << endl;
		fix_operators[op_no].dump();
		const vector<GlobalCondition> &conditions = fix_operators[op_no].get_preconditions();
		for (size_t cond_no = 0; cond_no < conditions.size(); cond_no++) {
			int var = conditions[cond_no].var;
			int val = conditions[cond_no].val;
			cout << "Consider precond with var: " << var << ", val: " << val << endl;

			while (var != current_node->switch_var) {
				if (current_node->default_generator == NULL) {
					current_node->default_generator = new SuccessorGeneratorSwitch(
							get_next_fix_var(current_node->switch_var));
				}

				current_node = (SuccessorGeneratorSwitch*) current_node->default_generator;
			}
			// Here: var == current_node->switch_var

			int next_fix_var_index = get_next_fix_var(current_node->switch_var);
			if (next_fix_var_index == -1) {
				if (current_node->generator_for_value[val] == NULL) {
					current_node->generator_for_value[val] = new SuccessorGeneratorGenerate();
				}
				((SuccessorGeneratorGenerate*) current_node->generator_for_value[val])->add_op(&fix_operators[op_no]);

			} else {
				if (current_node->generator_for_value[val] == NULL) {
					current_node->generator_for_value[val] = new SuccessorGeneratorSwitch(next_fix_var_index);
				}

				current_node = (SuccessorGeneratorSwitch*) current_node->generator_for_value[val];
				if (cond_no == (conditions.size() - 1)) {
					// This was the last cond.
					if (current_node->immediate_ops == NULL) {
						current_node->immediate_ops = new SuccessorGeneratorGenerate();
					}
					((SuccessorGeneratorGenerate*) current_node->immediate_ops)->add_op(&fix_operators[op_no]);
				}
			}
		}

		// After processing an op, start at root again
		current_node = fix_actions_successor_generator;
	}
}

int FixActionsSearch::get_next_fix_var(int curr_var) {
	while (curr_var < (int) g_variable_domain.size()) {
		curr_var++;
		if (attack_vars.find(curr_var) != attack_vars.end()) {
			return curr_var;
		}
	}
	return -1;
}

SearchStatus FixActionsSearch::step() {
	return IN_PROGRESS;
}

void FixActionsSearch::add_options_to_parser(OptionParser &parser) {
	SearchEngine::add_options_to_parser(parser);
}

SearchEngine *_parse(OptionParser &parser) {
	FixActionsSearch::add_options_to_parser(parser);
	Options opts = parser.parse();
	if (!parser.dry_run()) {
		return new FixActionsSearch(opts);
	}
	return NULL;
}

Plugin<SearchEngine> _plugin("fixsearch", _parse);
