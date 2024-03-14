#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include "../../option_parser_util.h"

class World {
public:
    explicit World(const std::vector<std::string> &statics, const std::vector<std::string> &types);
    size_t PredicateIndex(const std::string &object);
    const std::string &PredicateName(size_t index);
    size_t ObjectIndex(const std::string &object);
    const std::string &ObjectName(size_t index);
    const std::vector<std::vector<size_t>> Instantiations();
private:
    // Maps predicate name to its number of parameters
    std::unordered_map<std::string, size_t> predicates;
    // Maps static predicates to the objects which has it
    std::unordered_map<std::string, std::vector<size_t>> statics;
    std::unordered_map<std::string, std::vector<size_t>> types;
    std::vector<std::string> objects;
    std::vector<std::vector<size_t>> instantiations;
};

