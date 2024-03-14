#pragma once

#include <vector>
#include <cstddef>
#include <string>

std::vector<std::vector<size_t>> Comb(size_t N, size_t K);
std::vector<std::vector<size_t>> Cartesian (const std::vector<std::vector<size_t>>& in);
std::pair<std::string, std::vector<std::string>> SplitOperator(const std::string &s);
std::pair<std::string, std::vector<std::string>> SplitFact(std::string s);
std::string MetaOperatorName();
size_t ActionParameters(const std::string &name);
std::vector<std::vector<std::string>> FindInstantiations(const std::string &operator_name);
std::vector<std::string> FindAllObjects();
