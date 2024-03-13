#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include "../../option_parser_util.h"

class Action {
public:
    const std::string &Name();
    size_t Parameters();
private:
    std::string name;
    std::size_t parameters;
};

class Predicate {
public:
    const std::string &Name();
    size_t Parameters();
private:
    std::string name;
    std::size_t parameters;
};

class World {
public:
    explicit World(const std::vector<std::string> &statics);
    size_t PredicateIndex(const std::string &object);
    const std::string &PredicateName(size_t index);
    size_t ActionIndex(const std::string &object);
    const std::string &ActionName(size_t index);
    size_t ObjectIndex(const std::string &object);
    const std::string &ObjectName(size_t index);
    const std::vector<std::vector<size_t>> Instantiations();
private:
    std::unordered_map<std::string, size_t> predicates;
    std::vector<Action> actions;
    std::vector<std::string> objects;
    std::vector<std::vector<size_t>> instantiations;
};

