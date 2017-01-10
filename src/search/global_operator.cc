#include "global_operator.h"

#include "globals.h"

#include <iostream>
using namespace std;

GlobalCondition::GlobalCondition(istream &in) {
    in >> var >> val;
}

// TODO if the input file format has been changed, we would need something like this
// Effect::Effect(istream &in) {
//    int cond_count;
//    in >> cond_count;
//    for (int i = 0; i < cond_count; ++i)
//        cond.push_back(Condition(in));
//    in >> var >> post;
//}


void GlobalOperator::read_pre_post(istream &in) {
    int cond_count, var, pre, post;
    in >> cond_count;
    vector<GlobalCondition> conditions;
    conditions.reserve(cond_count);
    for (int i = 0; i < cond_count; ++i)
        conditions.push_back(GlobalCondition(in));
    in >> var >> pre >> post;
    if (pre != -1)
        preconditions.push_back(GlobalCondition(var, pre));
    effects.push_back(GlobalEffect(var, post, conditions));
}

GlobalOperator::GlobalOperator(istream &in, bool axiom, int cost_2)
: cost2(cost_2),
  conds_variable_name(&g_variable_name),
  effs_variable_name(&g_variable_name){
    marked = false;

    is_an_axiom = axiom;
    if (!is_an_axiom) {
        check_magic(in, "begin_operator");
        in >> ws;
        getline(in, name);
        int count;
        in >> count;
        for (int i = 0; i < count; ++i)
            preconditions.push_back(GlobalCondition(in));
        in >> count;
        for (int i = 0; i < count; ++i)
            read_pre_post(in);

        int op_cost;
        in >> op_cost;
        cost = g_use_metric ? op_cost : 1;

        g_min_action_cost = min(g_min_action_cost, cost);
        g_max_action_cost = max(g_max_action_cost, cost);

        check_magic(in, "end_operator");
    } else {
        name = "<axiom>";
        cost = 0;
        check_magic(in, "begin_rule");
        read_pre_post(in);
        check_magic(in, "end_rule");
    }

    marker1 = marker2 = false;
}

GlobalOperator::GlobalOperator(bool _is_an_axiom, std::vector<GlobalCondition> _preconditions, std::vector<GlobalEffect> _effects, std::string _name, int _cost, int _cost2, int _op_id, const std::vector<std::string> &_conds_variable_name, const std::vector<std::string> &_effs_variable_name, int _scheme_id)
: is_an_axiom(_is_an_axiom),
  preconditions(_preconditions),
  effects(_effects),
  name(_name),
  cost(_cost),
  cost2(_cost2),
  op_id(_op_id),
  conds_variable_name(&_conds_variable_name),
  effs_variable_name(&_effs_variable_name),
  scheme_id(_scheme_id){
	marked = false;
	marker1 = marker2 = false;
}

void GlobalCondition::dump(const vector<string> &conds_variable_name) const {
    cout << conds_variable_name[var] << ": " << val;
}

void GlobalEffect::dump(const vector<string> &conds_variable_name, const vector<string> &effs_variable_name) const {
    cout << effs_variable_name[var] << ":= " << val;
    if (!conditions.empty()) {
        cout << " if";
        for (size_t i = 0; i < conditions.size(); ++i) {
            cout << " ";
            conditions[i].dump(conds_variable_name);
        }
    }
}

void GlobalOperator::dump() const {
    cout << name << ":";
    for (size_t i = 0; i < preconditions.size(); ++i) {
        cout << " [";
        preconditions[i].dump(*conds_variable_name);
        cout << "]";
    }
    for (size_t i = 0; i < effects.size(); ++i) {
        cout << " [";
        effects[i].dump(*conds_variable_name, *effs_variable_name);
        cout << "]";
    }
    cout << endl;
}
