#include "stubborn_sets.h"

#include "../utils/collections.h"

#include <algorithm>
#include <cassert>

using namespace std;

namespace stubborn_sets {
// Relies on both fact sets being sorted by variable.
bool contain_conflicting_fact(const vector<std::pair <int,int>> &facts1,
                              const vector<std::pair <int,int>> &facts2) {
    auto facts1_it = facts1.begin();
    auto facts2_it = facts2.begin();
    while (facts1_it != facts1.end() && facts2_it != facts2.end()) {
        if (facts1_it->first < facts2_it->first) {
            ++facts1_it;
        } else if (facts1_it->first > facts2_it->first) {
            ++facts2_it;
        } else {
            if (facts1_it->second != facts2_it->second)
                return true;
            ++facts1_it;
            ++facts2_it;
        }
    }
    return false;
}

void StubbornSets::initialize() {
    PruningMethod::initialize();
    verify_no_axioms();
    verify_no_conditional_effects();

    num_operators = g_all_attack_operators.size();
    num_unpruned_successors_generated = 0;
    num_pruned_successors_generated = 0;
    sorted_goals = utils::sorted<pair <int,int>>(g_goal);

    compute_sorted_operators();
    compute_achievers();
}

// Relies on op_preconds and op_effects being sorted by variable.
bool StubbornSets::can_disable(int op1_no, int op2_no) const {
    return contain_conflicting_fact(sorted_op_effects[op1_no],
                                    sorted_op_preconditions[op2_no]);
}

// Relies on op_effect being sorted by variable.
bool StubbornSets::can_conflict(int op1_no, int op2_no) const {
    return contain_conflicting_fact(sorted_op_effects[op1_no],
                                    sorted_op_effects[op2_no]);
}

void StubbornSets::compute_sorted_operators() {
	sorted_op_preconditions.resize(g_all_attack_operators.size());
	sorted_op_effects.resize(g_all_attack_operators.size());

	for (size_t op_no = 0; op_no < g_all_attack_operators.size(); op_no++) {
		const vector<GlobalCondition> &preconditions = g_all_attack_operators[op_no].get_preconditions();
		const vector<GlobalEffect> &effects = g_all_attack_operators[op_no].get_effects();

		// Assuming here, that preconditions and effects are already sorted and that there are no conditional effects!

		vector<pair<int, int>> precondition_facts;
		for (size_t pre_no = 0; pre_no < preconditions.size(); pre_no++) {
			int var = preconditions[pre_no].var;
			int val = preconditions[pre_no].val;
			precondition_facts.push_back(pair<int, int> (var, val));
		}

		vector<pair<int, int>> effect_facts;
		for (size_t eff_no = 0; eff_no < effects.size(); eff_no++) {
			int var = effects[eff_no].var;
			int val = effects[eff_no].val;
			effect_facts.push_back(pair<int, int> (var, val));
		}

		sorted_op_preconditions[op_no] = precondition_facts;
		sorted_op_effects[op_no] = effect_facts;
	}
}

void StubbornSets::compute_achievers() {
	achievers.resize(g_variable_domain.size());

	for (size_t var_no = 0; var_no < g_variable_domain.size(); var_no++) {
		vector<vector<int>> temp (g_variable_domain[var_no], vector<int>());
		achievers[var_no] = temp;
	}

	for (size_t op_no = 0; op_no < g_all_attack_operators.size(); op_no++) {
		const vector<GlobalEffect> &effects = g_all_attack_operators[op_no].get_effects();

		for (size_t eff_no = 0; eff_no < effects.size(); eff_no++) {
			int var = effects[eff_no].var;
			int val = effects[eff_no].val;

			achievers[var][val].push_back(op_no);
		}
	}
}

bool StubbornSets::mark_as_stubborn(int op_no) {
    if (!stubborn[op_no]) {
        stubborn[op_no] = true;
        stubborn_queue.push_back(op_no);
        return true;
    }
    return false;
}

void StubbornSets::prune_operators(
    const GlobalState &state, vector<int> &op_ids) {
    num_unpruned_successors_generated += op_ids.size();

    // Clear stubborn set from previous call.
    stubborn.assign(num_operators, false);
    assert(stubborn_queue.empty());

    initialize_stubborn_set(state);
    /* Iteratively insert operators to stubborn according to the
       definition of strong stubborn sets until a fixpoint is reached. */
    while (!stubborn_queue.empty()) {
        int op_no = stubborn_queue.back();
        stubborn_queue.pop_back();
        handle_stubborn_operator(state, op_no);
    }

    // Now check which applicable operators are in the stubborn set.
    vector<int> remaining_op_ids;
    remaining_op_ids.reserve(op_ids.size());
    for (int op_id : op_ids) {
        if (stubborn[op_id]) {
            remaining_op_ids.push_back(op_id);
        }
    }
    op_ids.swap(remaining_op_ids);

    num_pruned_successors_generated += op_ids.size();
}

void StubbornSets::print_statistics() const {
    cout << "total successors before partial-order reduction: "
         << num_unpruned_successors_generated << endl
         << "total successors after partial-order reduction: "
         << num_pruned_successors_generated << endl;
}
}
