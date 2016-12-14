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

using namespace std;

vector<GlobalOperator> all_operators;
vector<GlobalOperator> fix_operators;
vector<GlobalOperator> attack_operators;

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

	// Sort the variables
	divideVariables();

	clean_attack_actions();

	g_operators.clear();

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
	for (size_t op_no = 0; g_operators.size(); op_no++) {
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
	    for(size_t i = 0; i < conditions.size(); i++) {
	    	int var = conditions[i].var;
	    	if(attack_vars.find(var) != attack_vars.end()) {
	    		// This is a precondition on an attack var
	    		attack_preconditions.push_back(conditions[i]);
	    	} else {
	    		// This is a precondition on a fix var
	    		fix_preconditions.push_back(conditions[i]);
	    	}
	    }
	    // TODO we need to do sth. with the fix_preconditions
	    GlobalOperator new_op(op.is_axiom(), attack_preconditions, op.get_effects(), op.get_name(), op.get_cost(), op.get_cost2());
	    attack_operators[op_no] = new_op;
	}
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
