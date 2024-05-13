#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include "../../option_parser_util.h"
#include "../../symbolic/sym_variables.h"

class s_hasher {
public:
    std::size_t operator()(std::vector<size_t> const& vec) const {
        std::size_t seed = vec.size();
        for(auto x : vec) {
            x = ((x >> 16) ^ x) * 0x45d9f3b;
            x = ((x >> 16) ^ x) * 0x45d9f3b;
            x = (x >> 16) ^ x;
            seed ^= x + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }

};

class World {
public:
    explicit World(
            const std::vector<std::string> &type_names,
            const std::vector<std::vector<std::string>> &type_objects,
            const std::vector<std::string> &static_names,
            const std::vector<std::vector<std::vector<std::string>>> &static_facts
            );
    size_t PredicateIndex(const std::string &predicate) const;
    const std::string &PredicateName(size_t index) const;
    size_t PredicateParameters(size_t index) const;
    size_t PredicateCount() const;
    bool IsStatic(size_t i) const;
    bool HasStatic(size_t predicate, std::vector<size_t> objects) const;
    size_t ObjectIndex(const std::string &object) const;
    size_t ObjectCount() const;
    const std::string &ObjectName(size_t index) const;
    bool IsObject(const std::string &object) const;
    size_t TypeIndex(const std::string &type) const;
    const std::string &TypeName(size_t index) const;
    size_t TypeCount() const;
    const std::vector<size_t> &TypeObjects(size_t index) const;
    size_t InstantiationParameters() const;
    size_t InstantiationCount() const;
    const std::vector<size_t> &Instantiation(size_t index) const;
    const BDD &InstantiationBDD(size_t index) const;
    const BDD* FactBDD(size_t predicate, const std::vector<size_t> &objects) const;

    void Init(const std::shared_ptr<symbolic::SymVariables> vars);
private:
    // Maps predicate name to its number of parameters
    std::vector<std::pair<std::string, size_t>> predicates;
    // Maps static predicates to the objects which has it
    std::vector<std::pair<std::string, std::vector<std::vector<size_t>>>> statics;
    std::vector<std::pair<std::string, std::vector<size_t>>> types;
    std::vector<std::string> objects;
    std::unordered_map<size_t, std::unordered_map<std::vector<size_t>, BDD, s_hasher>> fact_bdds;
    std::vector<std::vector<size_t>> instantiations;
    std::vector<BDD> instantiation_bdds;
};

