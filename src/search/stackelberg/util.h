#ifndef STACKELBERG_UTIL_H
#define STACKELBERG_UTIL_H

#include <random>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <numeric>

#include "../global_operator.h"
#include "../successor_generator.h"

namespace stackelberg {
inline std::string get_uuid() {
  static std::random_device dev;
  static std::mt19937 rng(dev());

  std::uniform_int_distribution<int> dist(0, 15);

  const char *v = "0123456789abcdef";
  const bool dash[] = {0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0};

  std::string res;
  for (int i = 0; i < 16; i++) {
    if (dash[i])
      res += "-";
    res += v[dist(rng)];
    res += v[dist(rng)];
  }
  return res;
}
inline std::vector<std::vector<size_t>> comb(size_t N, size_t K)
{
    std::string bitmask(K, 1); // K leading 1's
    bitmask.resize(N, 0); // N-K trailing 0's

    std::vector<std::vector<size_t>> combinations;
    do {
        std::vector<size_t> combination;
        for (size_t i = 0; i < N; ++i) // [0..N-1] integers
            if (bitmask[i])
                combination.push_back(i);
        combinations.push_back(std::move(combination));
    } while (std::prev_permutation(bitmask.begin(), bitmask.end()));

    return combinations;
}

inline std::vector<std::vector<size_t>> cartesian (const std::vector<std::vector<size_t>>& in) {
    std::vector<std::vector<size_t>> results = {{}};
    for (const auto& new_values : in) {
        std::vector<std::vector<size_t>> next_results;
        for (const auto& result : results) {
            for (const auto value : new_values) {
                next_results.push_back(result);
                next_results.back().push_back(value);
            }
        }
        results = std::move(next_results);
    }
    return results;
}

inline std::pair<std::string, std::vector<std::string>> split_operator(const std::string &s) {
    std::stringstream is(s);
    std::vector<std::string> objects;
    std::string name;
    std::string object;
    is >> std::skipws >> name;
    while (is >> std::skipws >> object) objects.push_back(object);
    return {name, objects};
}

/**
 * returns a SuccessorGeneratorSwitch based on the preconditions of the ops in
 * pre_cond_ops and entailing the ops from ops vector in the leaves
 */
inline SuccessorGeneratorSwitch *
create_successor_generator(const std::vector<int> &variable_domain,
                           const std::vector<GlobalOperator> &pre_cond_ops,
                           const std::vector<GlobalOperator> &ops) {
#ifdef LEADER_SEARCH_DEBUG
  std::cout << "Begin create_successor_generator..." << std::endl;
#endif

  int root_var_index = 0;

  auto root_node = new SuccessorGeneratorSwitch(
      root_var_index, variable_domain[root_var_index]);

  for (size_t op_no = 0; op_no < pre_cond_ops.size(); op_no++) {
    const auto &conditions = pre_cond_ops[op_no].get_preconditions();

    if (conditions.empty()) {
      // This op has no preconditions, add it immediately to the root node
      if (root_node->immediate_ops == nullptr) {
        root_node->immediate_ops = new SuccessorGeneratorGenerate();
      }
      ((SuccessorGeneratorGenerate *)root_node->immediate_ops)
          ->add_op(&ops[op_no]);
      continue;
    }

    SuccessorGeneratorSwitch *current_node = root_node;
    for (size_t cond_no = 0; cond_no < conditions.size(); cond_no++) {
      int var = conditions[cond_no].var;
      int val = conditions[cond_no].val;
      // cout << "Consider precond with var: " << var << ", val: " << val <<
      // endl;

      while (var != current_node->switch_var) {
        if (current_node->default_generator == nullptr) {
          int next_var_index = current_node->switch_var + 1;
          current_node->default_generator = new SuccessorGeneratorSwitch(
              next_var_index, variable_domain[next_var_index]);
        }

        current_node =
            (SuccessorGeneratorSwitch *)current_node->default_generator;
      }

      // Here: var == current_node->switch_var

      int next_var_index = current_node->switch_var + 1;
      if (next_var_index >= (int)variable_domain.size()) {
        if (current_node->generator_for_value[val] == nullptr) {
          current_node->generator_for_value[val] =
              new SuccessorGeneratorGenerate();
        }
        ((SuccessorGeneratorGenerate *)current_node->generator_for_value[val])
            ->add_op(&ops[op_no]);

      } else {
        if (current_node->generator_for_value[val] == nullptr) {
          current_node->generator_for_value[val] = new SuccessorGeneratorSwitch(
              next_var_index, variable_domain[next_var_index]);
        }

        current_node =
            (SuccessorGeneratorSwitch *)current_node->generator_for_value[val];
        if (cond_no == (conditions.size() - 1)) {
          // This was the last cond.
          if (current_node->immediate_ops == nullptr) {
            current_node->immediate_ops = new SuccessorGeneratorGenerate();
          }
          ((SuccessorGeneratorGenerate *)current_node->immediate_ops)
              ->add_op(&ops[op_no]);
        }
      }
    }
  }

  return root_node;
}

} // namespace stackelberg
#endif
