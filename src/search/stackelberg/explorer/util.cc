#include <algorithm>
#include <ios>
#include <sstream>
#include "util.h"

std::vector<std::vector<size_t>> comb(size_t N, size_t K)
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

std::vector<std::vector<size_t>> cartesian (const std::vector<std::vector<size_t>>& in) {
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

std::pair<std::string, std::vector<std::string>> split_operator(const std::string &s) {
  std::stringstream is(s);
  std::vector<std::string> objects;
  std::string name;
  std::string object;
  is >> std::skipws >> name;
  while (is >> std::skipws >> object) objects.push_back(object);
  return {name, objects};
}