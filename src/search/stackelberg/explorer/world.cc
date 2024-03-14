#include "world.h"
#include "../../plugin.h"
#include "util.h"

World::World(const std::vector<std::string> &statics, const std::vector<std::string> &types) {
    std::cout << "Generating World" << std::endl;
    this->objects = FindAllObjects();
    std::cout << "Objects: " << objects.size() << endl;
    for (const auto &var : g_fact_names)
        for (const auto &fact : var) {
            if (fact.find("is-goal") != std::string::npos)
                continue;
            if (fact.find("leader-state") != std::string::npos)
                continue;
            if (fact.find("leader-turn") != std::string::npos)
                continue;
            if (fact.find("none of those") != std::string::npos)
                continue;
            const auto p = SplitFact(fact);
            bool novel = true;
            for (const auto &predicate : this->predicates)
                if (predicate.first == p.first) {
                    novel = false;
                    break;
                }
            if (!novel) continue;
            this->predicates.emplace_back(p.first, p.second.size());
        }
    std::cout << "Predicates: " << predicates.size() << endl;
    for (const auto &s : statics) {
        std::stringstream is(s);
        std::vector<size_t> s_objects;
        std::string name;
        std::string object;
        is >> std::skipws >> name;
        while (is >> std::skipws >> object) s_objects.push_back(ObjectIndex(object));
        this->statics.emplace_back(name, s_objects);
    }
    std::cout << "Static predicates: " << statics.size() << endl;
    for (const auto &s : types) {
        std::stringstream is(s);
        std::vector<size_t> s_objects;
        std::string name;
        std::string object;
        is >> std::skipws >> name;
        while (is >> std::skipws >> object) s_objects.push_back(ObjectIndex(object));
        this->types.emplace_back(name, s_objects);
    }
    std::cout << "Types: " << types.size() << endl;
    if (this->types.empty()) {
        std::vector<size_t> idx;
        for (const auto &object: this->objects) idx.push_back(ObjectIndex(object));
        this->types.emplace_back("object", idx);
    }

}

size_t World::PredicateIndex(const std::string &predicate) const {
    for (size_t i = 0; i < this->predicates.size(); i++)
        if (predicates[i].first == predicate) return i;
    throw std::logic_error("Unknown predicate: " + predicate);
}
const std::string &World::PredicateName(size_t index) const {
    return predicates[index].first;
}

size_t World::PredicateParameters(size_t index) const {
    return predicates[index].second;
}

size_t World::PredicateCount() const {
    return predicates.size();
}

size_t World::ObjectIndex(const std::string &object) const {
    for (size_t i = 0; i < this->objects.size(); i++)
        if (objects[i] == object) return i;
    throw std::logic_error("Unknown object: " + object);
}

const std::string &World::ObjectName(size_t index) const {
    return objects[index];
}

size_t World::TypeIndex(const std::string &type) const {
    for (size_t i = 0; i < this->types.size(); i++)
        if (types[i].first == type) return i;
    throw std::logic_error("Unknown type: " + type);
}

const std::string &World::TypeName(size_t index) const {
    return this->types[index].first;
}

size_t World::TypeCount() const {
    return this->types.size();
}

const std::vector<size_t> &World::TypeObjects(size_t index) const {
    return this->types[index].second;
}

const BDD& World::FactBDD(size_t predicate, const std::vector<size_t> &objects) const {
    return fact_bdds.at(predicate).at(objects);
}

void World::Init(const std::shared_ptr<symbolic::SymVariables> vars) {
    for (size_t var = 0; var < g_fact_names.size(); var++)
        for (size_t val = 0; val < g_fact_names[var].size(); val++) {
            if (g_fact_names[var][val].find("is-goal") != std::string::npos)
                continue;
            if (g_fact_names[var][val].find("leader-state") != std::string::npos)
                continue;
            if (g_fact_names[var][val].find("leader-turn") != std::string::npos)
                continue;
            if (g_fact_names[var][val].find("none of those") != std::string::npos)
                continue;
            const auto p = SplitFact(g_fact_names[var][val]);
            const size_t predicate = PredicateIndex(p.first);
            std::vector<size_t> i_objects;
            for (const auto &o : p.second)
                i_objects.push_back(ObjectIndex(o));
            this->fact_bdds[predicate][i_objects] = vars->preBDD(var, val);
        }
}
