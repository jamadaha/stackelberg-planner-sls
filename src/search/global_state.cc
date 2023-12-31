#include "global_state.h"

#include "utilities.h"
#include "state_registry.h"

#include <algorithm>
#include <iostream>
#include <cassert>
using namespace std;


GlobalState::GlobalState(const PackedStateBin *buffer_,
                         const StateRegistry *registry_,
                         StateID id_,
                         IntPacker *_state_packer)
    : buffer(buffer_),
      registry(registry_),
      id(id_),
      state_packer(_state_packer)
{
    assert(buffer);
    assert(id != StateID::no_state);
}

GlobalState::~GlobalState()
{
}

int GlobalState::operator[](size_t index) const
{
    return state_packer->get(buffer, index);
}

void GlobalState::dump_pddl() const
{
    for (size_t i = 0; i < g_variable_domain.size(); ++i) {
        const string &fact_name = g_fact_names[i][(*this)[i]];
        if (fact_name != "<none of those>") {
            cout << fact_name << endl;
        }
    }
}

void GlobalState::dump_fdr() const
{
    for (size_t i = 0; i < g_variable_domain.size(); ++i)
        cout << "  #" << i << " [" << g_variable_name[i] << "] -> "
             << (*this)[i] << endl;
}

void GlobalState::dump_pddl(const vector<int> &variable_domain,
                            const vector<vector<string>> &fact_names) const
{
    for (size_t i = 0; i < variable_domain.size(); ++i) {
        const string &fact_name = fact_names[i][(*this)[i]];
        if (fact_name != "<none of those>") {
            cout << fact_name << endl;
        }
    }
}

void GlobalState::dump_fdr(const vector<int> &variable_domain,
                           const vector<string> &variable_name) const
{
    for (size_t i = 0; i < variable_domain.size(); ++i)
        cout << "  #" << i << " [" << variable_name[i] << "] -> "
             << (*this)[i] << endl;
}
