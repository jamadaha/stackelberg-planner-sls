#include "world.h"
#include "../../plugin.h"
#include "util.h"

World::World(
        const std::vector<std::string> &type_names,
        const std::vector<std::vector<std::string>> &type_objects,
        const std::vector<std::string> &static_names,
        const std::vector<std::vector<std::vector<std::string>>> &static_facts
) {
    std::cout << "Generating World" << std::endl;
    this->objects = FindAllObjects();
    std::cout << "Objects: " << objects << endl;
    cout << "Type names: " << type_names << endl;
    cout << "Type objects: " << type_objects << endl;
    assert(type_names.size() == type_objects.size());
    std::transform(
            type_names.begin(),
            type_names.end(),
            type_objects.begin(),
            std::back_inserter(this->types), [&](const auto &t_name, const auto &t_objects) {
                std::vector<size_t> indexes;
                indexes.reserve(t_objects.size());
                for (const auto &o : t_objects)
                    if (IsObject(o))
                        indexes.push_back(ObjectIndex(o));
                return make_pair(t_name, indexes);
    });
    this->types.erase(std::remove_if(
        this->types.begin(), this->types.end(),
        [](const auto &t) {
            return t.second.empty();
        }), this->types.end());
    cout << "Types: " << types << endl;
    cout << "Static names: " << static_names << endl;
    cout << "Static objects: " << static_facts << endl;
    assert(static_names.size() == static_facts.size());
    std::transform(
            static_names.begin(),
            static_names.end(),
            static_facts.begin(),
            std::back_inserter(this->statics), [&](const auto &s_name, const auto &s_facts) {
                std::vector<std::vector<size_t>> indexes;
                for (const auto &f : s_facts) {
                    std::vector<size_t> f_indexes;
                    f_indexes.reserve(f.size());
                    bool exists = true;
                    for (const auto &o: f)  {
                        if (!IsObject(o)) {
                            exists = false;
                            break;
                        }
                        f_indexes.push_back(ObjectIndex(o));
                    }
                    if (exists)
                        indexes.push_back(f_indexes);
                }
                return make_pair(s_name, indexes);
            });
    this->statics.erase(std::remove_if(
        this->statics.begin(), this->statics.end(),
        [](const auto &t) {
            return t.second.empty();
        }), this->statics.end());
    cout << "Statics: " << statics << endl;
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
    std::cout << "Predicates: " << predicates << endl;
}

size_t World::PredicateIndex(const std::string &predicate) const {
    for (size_t i = 0; i < this->predicates.size(); i++)
        if (predicates.at(i).first == predicate) return i;
    throw std::logic_error("Unknown predicate: " + predicate);
}
const std::string &World::PredicateName(size_t index) const {
    if (IsStatic(index))
        return statics.at(index - predicates.size()).first;
    else
        return predicates.at(index).first;
}

size_t World::PredicateParameters(size_t index) const {
    if (IsStatic(index))
        return statics.at(index - predicates.size()).second.begin()->size();
    else
        return predicates.at(index).second;
}

size_t World::PredicateCount() const {
    return predicates.size() + statics.size();
}

bool World::IsStatic(size_t i) const {
    return i >= predicates.size();
}

bool World::HasStatic(size_t predicate, std::vector<size_t> objects) const {
    const size_t p_index = predicate - predicates.size();
    for (size_t i = 0; i < statics.at(p_index).second.size(); i++)
        if (objects == statics.at(p_index).second.at(i)) return true;
    return false;
}

size_t World::ObjectIndex(const std::string &object) const {
    for (size_t i = 0; i < this->objects.size(); i++)
        if (objects.at(i) == object) return i;
    throw std::logic_error("Unknown object: " + object);
}

const std::string &World::ObjectName(size_t index) const {
    return objects.at(index);
}
size_t World::ObjectCount() const {
    return this->objects.size();
}


bool World::IsObject(const std::string &object) const {
    for (const auto &o : objects)
        if (object == o) return true;
    return false;
}

size_t World::TypeIndex(const std::string &type) const {
    for (size_t i = 0; i < this->types.size(); i++)
        if (types.at(i).first == type) return i;
    throw std::logic_error("Unknown type: " + type);
}

const std::string &World::TypeName(size_t index) const {
    return this->types.at(index).first;
}

size_t World::TypeCount() const {
    return this->types.size();
}

const std::vector<size_t> &World::TypeObjects(size_t index) const {
    return this->types.at(index).second;
}

const BDD* World::FactBDD(size_t predicate, const std::vector<size_t> &objects) const {
    if (fact_bdds.find(predicate) == fact_bdds.end()) return nullptr;
    if (fact_bdds.at(predicate).find(objects) == fact_bdds.at(predicate).end()) return nullptr;
    return &fact_bdds.at(predicate).at(objects);
}

void World::Init(const std::shared_ptr<symbolic::SymVariables> vars) {
    printf("Initializing world...\n");
    for (size_t var = 0; var < g_fact_names.size(); var++)
        for (size_t val = 0; val < g_fact_names.at(var).size(); val++) {
            if (g_fact_names[var][val].find("is-goal") != std::string::npos)
                continue;
            if (g_fact_names[var][val].find("leader-state") != std::string::npos)
                continue;
            if (g_fact_names[var][val].find("leader-turn") != std::string::npos)
                continue;
            if (g_fact_names[var][val].find("none of those") != std::string::npos)
                continue;
            const auto p = SplitFact(g_fact_names.at(var).at(val));
            const size_t predicate = PredicateIndex(p.first);
            std::vector<size_t> i_objects;
            for (const auto &o : p.second)
                i_objects.push_back(ObjectIndex(o));
            this->fact_bdds[predicate][i_objects] = vars->preBDD(var, val);
        }
    printf("Finished initializing world.\n");
}
