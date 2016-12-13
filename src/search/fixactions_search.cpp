/*
 * fixactionssearch.cpp
 *
 *  Created on: 12.12.2016
 *      Author: Patrick
 */

#include "fixactions_search.h"
#include <vector>
#include <math.h>

using namespace std;

vector<GlobalOperator> all_operators;
vector<GlobalOperator> fix_operators;


FixActionsSearch::FixActionsSearch(const Options &opts)
: SearchEngine(opts) {
	// TODO Auto-generated constructor stub

}

FixActionsSearch::~FixActionsSearch() {
	// TODO Auto-generated destructor stub
}

void FixActionsSearch::initialize() {
	vector<GlobalOperator> new_g_operators;

	// Sort the operators
	for (size_t op = 0; g_operators.size(); op++) {
		string op_name = g_operators[op].name;
		if(op_name.find("ATTACK") == 0) {
			new_g_operators.push_back(g_operators[op]);
			all_operators.push_back(g_operators[op]);

			size_t underscore = op_name.find_last_of("_");
			if(underscore == string::npos) {
				cout << "No success probability suffix found! Error in PDDL file?" << endl;
				exit(EXIT_INPUT_ERROR);
			}

			string prob = op_name.substr(underscore + 1);
			size_t backslash = prob.find("/");
			if(backslash == string::npos) {
				cout << "No correct success probability suffix found! Error in PDDL file?" << endl;
				exit(EXIT_INPUT_ERROR);
			}

			string numerator_string = prob.substr(0, backslash);
			string denominator_string = prob.substr(backslash + 1);
			double numerator = (double) stoi(numerator_string);
			double denominator = (double) stoi(denominator_string);
			int success_prob_cost = (int) (fabs(log2(numerator/denominator)) * 1000);
		} else if(op_name.find("FIX") == 0) {
			fix_operators.push_back(g_operators[op]);
			all_operators.push_back(g_operators[op]);

			size_t underscore = op_name.find_last_of("_");
			if(underscore == string::npos) {
				cout << "No success probability suffix found! Error in PDDL file?" << endl;
				exit(EXIT_INPUT_ERROR);
			}
			string invention_cost_string = op_name.substr(underscore + 1);
			int invention_cost = stoi(invention_cost_string);
		} else {
			cout << "No op prefix found! Error in PDDL file?" << endl;
			exit(EXIT_INPUT_ERROR);
		}
	}

	// Sort the variables
	unordered_set<int> attack_vars;
	for (size_t op = 0; g_operators.size(); op++) {
		const vector<GlobalEffect> &effects = g_operators[op].get_effects();
		for (size_t i = 0; i < effects.size(); i++) {
			int var = effects[i].var;
			attack_vars.insert(var);
		}

	}


}

SearchStatus FixActionsSearch::step() {
	return IN_PROGRESS;
}

