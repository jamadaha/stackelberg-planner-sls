#include "dominance_stackelberg_task.h"

#include "stackelberg_task.h"
#include <map>

using namespace std;

namespace stackelberg {

    DominanceStackelbergVariable::
    DominanceStackelbergVariable(int variable,
                                 const std::set<int> & values_in_precondition) : var(variable) {
        rel.resize(g_variable_domain[variable]);
        for (int val = 0; val < g_variable_domain[variable]; ++val) {
            rel[val].resize(g_variable_domain[variable], true);

            for (int val2 : values_in_precondition) {
                if (val != val2) {
                    rel[val][val2] = false;
                }
            }
        }
        
    }
        
    DominanceStackelbergTask::DominanceStackelbergTask(const StackelbergTask & task) {

        map<int, set<int>> values_in_precondition; 
        for (int op_no : task.get_global_operator_id_follower_ops()) {
            const GlobalOperator & op = g_operators[op_no];

            for (const auto & prevail : op.get_preconditions()) { 
                if (!task.is_follower_static_var(prevail.var)) {
                    continue;
                }
                values_in_precondition[prevail.var].insert(prevail.val);
            }                                                       
        }

        for (const auto & entry : values_in_precondition) {
            dominance.push_back(DominanceStackelbergVariable(entry.first, entry.second));
        }
    }
    
    bool DominanceStackelbergTask::dominates(const std::vector<int> & t,
                                               const std::vector<int> & s) const {
        for (const auto & dom : dominance) {
            if (!dom.dominates(t, s)) {
                return false;
            }
        }
        return true;
    }

                    
    bool DominanceStackelbergVariable::dominates(const std::vector<int> & t, const std::vector<int> & s) const{
        assert (var >= 0);
        assert ((size_t)var < t.size());
        assert (s.size() == t.size());
        assert (t[var] >= 0);
        assert (s[var] >= 0);
        assert ((size_t)(t[var]) < rel.size());
        assert ((size_t)(s[var]) < rel[t[var]].size());
        return rel[t[var]][s[var]];
    }

}
