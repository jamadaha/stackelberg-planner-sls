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
            this->predicates[p.first] = p.second.size();
        }
    std::cout << "Predicates: " << predicates.size() << endl;
    for (const auto &s : statics) {
        std::stringstream is(s);
        std::vector<size_t> s_objects;
        std::string name;
        std::string object;
        is >> std::skipws >> name;
        while (is >> std::skipws >> object) s_objects.push_back(ObjectIndex(object));
        this->statics[name] = s_objects;
    }
    std::cout << "Static predicates: " << statics.size() << endl;
    for (const auto &s : types) {
        std::stringstream is(s);
        std::vector<size_t> s_objects;
        std::string name;
        std::string object;
        is >> std::skipws >> name;
        while (is >> std::skipws >> object) s_objects.push_back(ObjectIndex(object));
        this->types[name] = s_objects;
    }
    std::cout << "Types: " << types.size() << endl;
}

size_t World::ObjectIndex(const std::string &object) {
    for (size_t i = 0; i < this->objects.size(); i++)
        if (objects[i] == object) return i;
    throw std::logic_error("Unknown object: " + object);
}
