#include <vector>
#include <cstddef>
#include <string>

std::vector<std::vector<size_t>> comb(size_t N, size_t K);
std::vector<std::vector<size_t>> cartesian (const std::vector<std::vector<size_t>>& in);
std::pair<std::string, std::vector<std::string>> split_operator(const std::string &s);