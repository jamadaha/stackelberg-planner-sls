#ifndef PER_FIX_STATE_INFORMATION_H
#define PER_FIX_STATE_INFORMATION_H

#include "segmented_vector.h"
#include "global_state.h"

#include <vector>
#include <unordered_set>

template<typename T>
class PerFixStateInformation
{
    struct HashState {
        const std::vector<int> &m_relevant_variables;
        const SegmentedVector<GlobalState> &states;
        HashState(const std::vector<int> &vars,
                  const SegmentedVector<GlobalState> &states)
            : m_relevant_variables(vars), states(states) {}
        size_t operator()(const unsigned &i) const
        {
            const GlobalState &state = states[i];
            // hash function adapted from Python's hash function for tuples.
            size_t hash_value = 0x345678;
            size_t mult = 1000003;
            for (int i = m_relevant_variables.size() - 1; i >= 0; --i) {
                hash_value = (hash_value ^ state[m_relevant_variables[i]]) * mult;
                mult += 82520 + i + i;
            }
            hash_value += 97531;
            return hash_value;
        }
    };
    struct CompareState {
        const std::vector<int> &m_relevant_variables;
        const SegmentedVector<GlobalState> &states;
        CompareState(const std::vector<int> &vars,
                     const SegmentedVector<GlobalState> &states)
            : m_relevant_variables(vars),
              states(states) {}
        bool operator()(const unsigned &i, const unsigned &j) const
        {
            const GlobalState &x = states[i];
            const GlobalState &y = states[j];
            for (int i = m_relevant_variables.size() - 1; i >= 0; --i) {
                const int &var = m_relevant_variables[i];
                if (x[var] != y[var]) {
                    return false;
                }
            }
            return true;
        }
    };
    std::vector<int> m_relevant_variables;
    SegmentedVector<GlobalState> m_states;
    std::unordered_set<unsigned, HashState, CompareState> m_ids;
    SegmentedVector<T> m_data;
public:
    PerFixStateInformation()
        : m_ids(0, HashState(m_relevant_variables, m_states),
                CompareState(m_relevant_variables, m_states))
    {}
    void set_relevant_variables(const std::vector<int> &vars)
    {
        m_relevant_variables.insert(m_relevant_variables.end(),
                                    vars.begin(),
                                    vars.end());
    }
    T &operator[](const unsigned &i)
    {
        return m_data[i];
    }
    T &operator[](const GlobalState &state)
    {
        m_states.push_back(state);
        auto it = m_ids.insert(m_states.size() - 1);
        if (!it.second) {
            m_states.pop_back();
        } else {
            m_data.push_back(T());
        }
        return m_data[*(it.first)];
    }
    void clear()
    {
        m_states.resize(0);
        m_ids.clear();
        m_data.resize(0);
    }
};

#endif
