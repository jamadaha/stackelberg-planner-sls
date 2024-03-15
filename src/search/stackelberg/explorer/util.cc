#include <algorithm>
#include <ios>
#include <sstream>
#include "util.h"
#include "../../globals.h"
#include "../../global_operator.h"

std::vector<std::vector<size_t>> Comb(size_t N, size_t K)
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

std::vector<std::vector<size_t>> Comb(size_t N, size_t min, size_t max) {
    std::vector<std::vector<size_t>> combs;
    for (size_t i = min; i <= max; i++) {
        const auto comb = Comb(N, i);
        combs.insert(combs.end(), comb.begin(), comb.end());
    }
    return combs;
}

std::vector<std::vector<size_t>> Cartesian (const std::vector<std::vector<size_t>>& in) {
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


std::vector<std::vector<size_t>> Cartesian (size_t n, size_t v) {
    std::vector<std::vector<size_t>> inputs;
    inputs.reserve(n);
    for (size_t i = 0; i < n; i++) {
        std::vector<size_t> input;
        for (size_t t = 0; t < v; t++)
            input.push_back(t);
        inputs.push_back(input);
    }
    return Cartesian(inputs);
}

std::pair<std::string, std::vector<std::string>> SplitOperator(const std::string &s) {
  std::stringstream is(s);
  std::vector<std::string> objects;
  std::string name;
  std::string object;
  is >> std::skipws >> name;
  while (is >> std::skipws >> object) objects.push_back(object);
  return {name, objects};
}

std::pair<std::string, std::vector<std::string>> SplitFact(std::string s) {
  size_t i = s.find('(');
  std::string name = s.substr(0, i);
  s = s.substr(i + 1);
  std::vector<std::string> objects;
  while (s.size() > 1) {
    i = std::min(s.find(','), s.find(')'));
    objects.push_back(s.substr(0, i));
    s = s.substr(i + 1);
    if (s[0] == ' ')
      s = s.substr(1);
  }
  return {name, objects};
}

std::string MetaOperatorName() {
  for (const auto &o : g_operators) {
    const auto name = o.get_name();
    if (std::find(name.begin(), name.end(), '$') != name.end())
      return SplitOperator(name).first;
  }
  throw std::logic_error("No meta operator define. (Any operator with $ in name)");
}

size_t ActionParameters(const std::string &name) {
  for (const auto &o : g_operators) {
    const auto p = SplitOperator(o.get_name());
    if (name == p.first) return p.second.size();
  }
  throw std::invalid_argument("No such action");
}

std::vector<std::vector<std::string>> FindInstantiations(const std::string &operator_name) {
  std::vector<std::vector<std::string>> instantiations;
  for (const auto &o : g_operators) {
    const auto name = o.get_name();
    const auto p = SplitOperator(name);
    if (p.first != operator_name) continue;
    instantiations.push_back(p.second);
  }
  return instantiations;
}

std::vector<std::string> FindAllObjects() {
  std::vector<std::string> objects;
  for (const auto &o : g_operators) {
    const auto o_objects = SplitOperator(o.get_name()).second;
    for (const auto &object : o_objects) {
      if (std::find(objects.begin(), objects.end(), object) == objects.end())
        objects.push_back(object);
    }
  }
  return objects;
}