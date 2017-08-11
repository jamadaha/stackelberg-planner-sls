#include "pattern_database.h"
#include "match_tree.h"

#include "../../globals.h"
#include "../../operator.h"
#include "../../state.h"

#include "../../utils/priority_queue.h"

#include "../../utils/collections.h"
#include "../../utils/math.h"
#include "../../utils/timer.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

using namespace std;

namespace std
{
template<typename T>
ostream &operator<<(ostream &out, const std::vector<T> &x)
{
    out << "[";
    for (unsigned i = 0; i < x.size(); i++) {
        out << (i > 0 ? ", " : "")
            << x[i];
    }
    out << "]";
    return out;
}
}

namespace pdbs
{
AbstractOperator::AbstractOperator(const vector<pair<int, int>> &prev_pairs,
                                   const vector<pair<int, int>> &pre_pairs,
                                   const vector<pair<int, int>> &eff_pairs,
                                   int cost,
                                   const vector<size_t> &hash_multipliers)
    : cost(cost),
      regression_preconditions(prev_pairs),
      progression_preconditions(prev_pairs)
{
    regression_preconditions.insert(regression_preconditions.end(),
                                    eff_pairs.begin(),
                                    eff_pairs.end());
    // Sort preconditions for MatchTree construction.
    sort(regression_preconditions.begin(), regression_preconditions.end());
    for (size_t i = 1; i < regression_preconditions.size(); ++i) {
        assert(regression_preconditions[i].first !=
               regression_preconditions[i - 1].first);
    }

    progression_preconditions.insert(progression_preconditions.end(),
                                     pre_pairs.begin(),
                                     pre_pairs.end());
    // Sort preconditions for MatchTree construction.
    sort(progression_preconditions.begin(), progression_preconditions.end());
    for (size_t i = 1; i < progression_preconditions.size(); ++i) {
        assert(progression_preconditions[i].first !=
               progression_preconditions[i - 1].first);
    }


    hash_effect = 0;
    assert(pre_pairs.size() == eff_pairs.size());
    for (size_t i = 0; i < pre_pairs.size(); ++i) {
        int var = pre_pairs[i].first;
        assert(var == eff_pairs[i].first);
        int old_val = eff_pairs[i].second;
        int new_val = pre_pairs[i].second;
        assert(new_val != -1);
        size_t effect = (new_val - old_val) * hash_multipliers[var];
        hash_effect += effect;
    }
}

AbstractOperator::~AbstractOperator()
{
}

void AbstractOperator::dump(const Pattern &pattern) const
{
    cout << "AbstractOperator:" << endl;
    cout << "Regression preconditions:" << endl;
    for (size_t i = 0; i < regression_preconditions.size(); ++i) {
        int var_id = regression_preconditions[i].first;
        int val = regression_preconditions[i].second;
        cout << "Variable: " << var_id << " (True name: "
             << g_variable_name[pattern[var_id]]
             << ", Index: " << i << ") Value: " << val << endl;
    }
    cout << "Hash effect:" << hash_effect << endl;
}

PatternDatabase::PatternDatabase(
    const Pattern &pattern,
    bool dump,
    bool compute_reachability,
    const vector<int> &operator_costs)
    : pattern(pattern)
{
    ::verify_no_axioms();
    ::verify_no_conditional_effects();
    assert(operator_costs.empty() ||
           operator_costs.size() == g_operators.size());
    assert(utils::is_sorted_unique(pattern));

    utils::Timer timer;
    hash_multipliers.reserve(pattern.size());
    num_states = 1;
    for (int pattern_var_id : pattern) {
        hash_multipliers.push_back(num_states);
        if (utils::is_product_within_limit(num_states,
                                           g_variable_domain[pattern_var_id],
                                           numeric_limits<int>::max())) {
            num_states *= g_variable_domain[pattern_var_id];
        } else {
            cerr << "Given pattern is too large! (Overflow occured): " << endl;
            cerr << pattern << endl;
            exit_with(EXIT_CRITICAL_ERROR);
        }
    }
    create_pdb(operator_costs, compute_reachability);
    if (dump) {
        cout << "PDB construction time: " << timer << endl;
    }
}

void PatternDatabase::multiply_out(
    int pos, int cost, vector<pair<int, int>> &prev_pairs,
    vector<pair<int, int>> &pre_pairs,
    vector<pair<int, int>> &eff_pairs,
    const vector<pair<int, int>> &effects_without_pre,
    vector<AbstractOperator> &operators)
{
    if (pos == static_cast<int>(effects_without_pre.size())) {
        // All effects without precondition have been checked: insert op.
        if (!eff_pairs.empty()) {
            operators.push_back(
                AbstractOperator(prev_pairs, pre_pairs, eff_pairs, cost,
                                 hash_multipliers));
        }
    } else {
        // For each possible value for the current variable, build an
        // abstract operator.
        int var_id = effects_without_pre[pos].first;
        int eff = effects_without_pre[pos].second;
        unsigned var = pattern[var_id];
        for (int i = 0; i < g_variable_domain[var]; ++i) {
            if (i != eff) {
                pre_pairs.push_back(make_pair(var_id, i));
                eff_pairs.push_back(make_pair(var_id, eff));
            } else {
                prev_pairs.push_back(make_pair(var_id, i));
            }
            multiply_out(pos + 1, cost, prev_pairs, pre_pairs, eff_pairs,
                         effects_without_pre, operators);
            if (i != eff) {
                pre_pairs.pop_back();
                eff_pairs.pop_back();
            } else {
                prev_pairs.pop_back();
            }
        }
    }
}

void PatternDatabase::build_abstract_operators(
    const Operator &op, int cost,
    const std::vector<int> &variable_to_index,
    vector<AbstractOperator> &operators)
{
    // All variable value pairs that are a prevail condition
    vector<pair<int, int>> prev_pairs;
    // All variable value pairs that are a precondition (value != -1)
    vector<pair<int, int>> pre_pairs;
    // All variable value pairs that are an effect
    vector<pair<int, int>> eff_pairs;
    // All variable value pairs that are a precondition (value = -1)
    vector<pair<int, int>> effects_without_pre;

    for (const auto &pre : op.get_prevail()) {
        int pattern_var_id = variable_to_index[pre.var];
        if (pattern_var_id != -1) {
            prev_pairs.push_back(make_pair(pattern_var_id, pre.prev));
        }
    }

    for (const auto &eff : op.get_pre_post()) {
        int pattern_var_id = variable_to_index[eff.var];
        if (pattern_var_id != -1) {
            if (eff.pre != -1) {
                pre_pairs.push_back(make_pair(pattern_var_id, eff.pre));
                eff_pairs.push_back(make_pair(pattern_var_id, eff.post));
            } else {
                effects_without_pre.push_back(make_pair(pattern_var_id, eff.post));
            }
        }
    }

    multiply_out(0, cost, prev_pairs, pre_pairs, eff_pairs, effects_without_pre,
                 operators);
}

void PatternDatabase::create_pdb(const std::vector<int> &operator_costs,
                                 bool compute_reachability)
{
    vector<int> variable_to_index(g_variable_domain.size(), -1);
    for (size_t i = 0; i < pattern.size(); ++i) {
        variable_to_index[pattern[i]] = i;
    }

    // compute all abstract operators
    vector<AbstractOperator> operators;
    for (const Operator &op : g_operators) {
        int op_cost;
        if (operator_costs.empty()) {
            op_cost = op.get_cost();
        } else {
            op_cost = operator_costs[op.get_id()];
        }
        build_abstract_operators(op, op_cost, variable_to_index, operators);
    }

    // build regression match tree
    MatchTree regression_match_tree(pattern, hash_multipliers);
    for (const AbstractOperator &op : operators) {
        regression_match_tree.insert(op, op.get_regression_preconditions());
    }

    // compute abstract goal var-val pairs
    vector<pair<int, int>> abstract_goals;
    for (const std::pair<int, int> &goal : g_goal) {
        if (variable_to_index[goal.first] != -1) {
            abstract_goals.push_back(make_pair(variable_to_index[goal.first], goal.second));
        }
    }

    distances.reserve(num_states);
    // first implicit entry: priority, second entry: index for an abstract state
    AdaptiveQueue<size_t> pq;

    // initialize queue
    for (size_t state_index = 0; state_index < num_states; ++state_index) {
        if (is_goal_state(state_index, abstract_goals)) {
            pq.push(0, state_index);
            distances.push_back(0);
        } else {
            distances.push_back(numeric_limits<int>::max());
        }
    }

    // Dijkstra loop
    while (!pq.empty()) {
        pair<int, size_t> node = pq.pop();
        int distance = node.first;
        size_t state_index = node.second;
        if (distance > distances[state_index]) {
            continue;
        }

        // regress abstract_state
        vector<const AbstractOperator *> applicable_operators;
        regression_match_tree.get_applicable_operators(state_index,
                applicable_operators);
        for (const AbstractOperator *op : applicable_operators) {
            size_t predecessor = state_index + op->get_hash_effect();
            int alternative_cost = distances[state_index] + op->get_cost();
            if (alternative_cost < distances[predecessor]) {
                distances[predecessor] = alternative_cost;
                pq.push(alternative_cost, predecessor);
            }
        }
    }

    if (compute_reachability) {
        compute_reachable_states(operators);
    }
}

bool PatternDatabase::is_goal_state(
    const size_t state_index,
    const vector<pair<int, int>> &abstract_goals) const
{
    for (pair<int, int> abstract_goal : abstract_goals) {
        int pattern_var_id = abstract_goal.first;
        int var_id = pattern[pattern_var_id];
        int temp = state_index / hash_multipliers[pattern_var_id];
        int val = temp % g_variable_domain[var_id];
        if (val != abstract_goal.second) {
            return false;
        }
    }
    return true;
}

size_t PatternDatabase::hash_index(const State &state) const
{
    size_t index = 0;
    for (size_t i = 0; i < pattern.size(); ++i) {
        index += hash_multipliers[i] * state[pattern[i]];
    }
    return index;
}

int PatternDatabase::get_value(const State &state) const
{
    return distances[hash_index(state)];
}

double PatternDatabase::compute_mean_finite_h() const
{
    double sum = 0;
    int size = 0;
    for (size_t i = 0; i < distances.size(); ++i) {
        if (distances[i] != numeric_limits<int>::max()) {
            sum += distances[i];
            ++size;
        }
    }
    if (size == 0) { // All states are dead ends.
        return numeric_limits<double>::infinity();
    } else {
        return sum / size;
    }
}

bool PatternDatabase::is_operator_relevant(const Operator &op) const
{
    for (const auto &effect : op.get_pre_post()) {
        if (binary_search(pattern.begin(), pattern.end(), effect.var)) {
            return true;
        }
    }
    return false;
}

vector<vector<std::pair<int, int> >> PatternDatabase::get_dead_ends() const
{
    vector<vector<std::pair<int, int> >> dead;
    for (size_t index = 0; index < distances.size(); ++index) {
        if (distances[index] != numeric_limits<int>::max()) {
            continue;
        }
        if (!reachable.empty() && !reachable[index]) {
            continue;
        }
        // Reverse index to partial state
        vector<std::pair<int, int> > partial_state;
        int remaining_index = index;
        for (int i = pattern.size() - 1; i >= 0; --i) {
            int value = remaining_index / hash_multipliers[i];
            partial_state.emplace_back(pattern[i], value);
            remaining_index -= value * hash_multipliers[i];
        }
        reverse(partial_state.begin(), partial_state.end());
        dead.push_back(partial_state);
    }
    return dead;
}

void PatternDatabase::compute_reachable_states(const vector<AbstractOperator>
        &operators)
{
    // build progression match tree
    MatchTree progression_match_tree(pattern, hash_multipliers);
    for (const AbstractOperator &op : operators) {
        progression_match_tree.insert(op, op.get_progression_preconditions());
    }

    // reachbility analysis
    vector<int> queue;
    reachable = vector<bool>(num_states, false);
    int initial_state_id = hash_index(g_initial_state());
    queue.push_back(initial_state_id);
    while (!queue.empty()) {
        int index = queue.back();
        queue.pop_back();
        if (reachable[index]) {
            continue;
        }
        reachable[index] = true;

        // Assume that this state is pruned and no successors are generated for it
        if (distances[index] == numeric_limits<int>::max()) {
            continue;
        }

        vector<const AbstractOperator *> applicable_operators;
        progression_match_tree.get_applicable_operators(index, applicable_operators);
        for (const AbstractOperator *op : applicable_operators) {
            int successor = index - op->get_hash_effect();
            queue.push_back(successor);
        }
    }
}
}
