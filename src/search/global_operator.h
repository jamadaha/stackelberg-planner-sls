#ifndef OPERATOR_H
#define OPERATOR_H

#include "global_state.h"
#include "globals.h"
#include "utilities.h"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

struct GlobalCondition {
    int var;
    int val;
    explicit GlobalCondition(std::istream &in);
    GlobalCondition(int variable, int value) : var(variable), val(value) {
        assert(in_bounds(var, g_variable_name));
        assert(val >= 0 && val < g_variable_domain[var]);
    }

    bool is_applicable(const GlobalState &state) const {
        return state[var] == val;
    }

    bool is_applicable(const std::vector<int> &state) const {
        return state[var] == val;
    }

    bool operator==(const GlobalCondition &other) const {
        return var == other.var && val == other.val;
    }

    bool operator!=(const GlobalCondition &other) const {
        return !(*this == other);
    }

    void dump(const std::vector<std::string> &conds_variable_name = g_variable_name) const;
};

struct GlobalEffect {
    int var;
    int val;
    std::vector<GlobalCondition> conditions;
    explicit GlobalEffect(std::istream &in);
    GlobalEffect(int variable, int value, const std::vector<GlobalCondition> &conds)
        : var(variable), val(value), conditions(conds) {}

    
    bool does_fire(const GlobalState &state) const {
        for (size_t i = 0; i < conditions.size(); ++i)
            if (!conditions[i].is_applicable(state))
                return false;
        return true;
    }
    
    bool does_fire(const std::vector<int> &state) const {
        for (size_t i = 0; i < conditions.size(); ++i)
            if (!conditions[i].is_applicable(state))
                return false;
        return true;
    }

    void dump(const std::vector<std::string> &conds_variable_name = g_variable_name, const std::vector<std::string> &effs_variable_name = g_variable_name) const;
};

class GlobalOperator {
    bool is_an_axiom;
    std::vector<GlobalCondition> preconditions;
    std::vector<GlobalEffect> effects;
    std::string name;
    int cost = -1;
    int cost2 = -1;
    int op_id = -1;
    const std::vector<std::string> *conds_variable_name;
    const std::vector<std::string> *effs_variable_name;
    int scheme_id = -1;

    mutable bool marked; // Used for short-term marking of preferred operators
    void read_pre_post(std::istream &in);
public:
    explicit GlobalOperator(std::istream &in, bool is_axiom, int cost2 = 0);
    explicit GlobalOperator(bool is_an_axiom, std::vector<GlobalCondition> preconditions, std::vector<GlobalEffect> effects, std::string name, int cost, int cost2, int op_id, const std::vector<std::string> &conds_variable_name = g_variable_name, const std::vector<std::string> &effs_variable_name = g_variable_name, int scheme_id = 0);

    void dump() const;
    std::string get_name() const {return name; }

    bool is_axiom() const {return is_an_axiom; }

    const std::vector<GlobalCondition> &get_preconditions() const {return preconditions; }
    const std::vector<GlobalEffect> &get_effects() const {return effects; }

    std::vector<GlobalCondition> &get_preconditions() {return preconditions; }
    std::vector<GlobalEffect> &get_effects() {return effects; }

    bool is_applicable(const GlobalState &state) const {
        for (size_t i = 0; i < preconditions.size(); ++i)
            if (!preconditions[i].is_applicable(state))
                return false;
        return true;
    }

    bool is_marked() const {
        return marked;
    }
    void mark() const {
        marked = true;
    }
    void unmark() const {
        marked = false;
    }

    mutable bool marker1, marker2; // HACK! HACK!

    int get_cost() const {return cost; }
    void set_cost(int _cost) {cost = _cost; }
    int get_cost2() const {return cost2; }
    void set_cost2(int _cost2) {cost2 = _cost2; }
    int get_op_id() const {return op_id; }
    void set_op_id(int _op_id) {op_id = _op_id; }
    void set_conds_variable_name (const std::vector<std::string> &_conds_variable_name) {conds_variable_name = &_conds_variable_name; }
    void set_effs_variable_name(const std::vector<std::string> &_effs_variable_name) {effs_variable_name = &_effs_variable_name; }
    int get_scheme_id() const {return scheme_id; }
    void set_scheme_id(int _scheme_id) {scheme_id = _scheme_id; }
};

#endif
