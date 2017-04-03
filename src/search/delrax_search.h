
#ifndef DELETE_RELAXATION_SEARCH_H
#define DELETE_RELAXATION_SEARCH_H

#include "search_engine.h"

#include <vector>
#include <deque>

namespace delrax_search
{

bool is_task_delete_free();

class DelRaxSearch : public ::SearchEngine
{
protected:
    const bool c_preprocess;

    bool m_dirty;

    bool initialized = false;

    std::vector<int> m_positive_values;
    std::vector<int> m_reward;
    std::vector<unsigned> m_goal;

    std::vector<unsigned> m_num_preconditions;
    std::vector<std::vector<unsigned> > m_in_pre;
    std::vector<std::vector<unsigned> > m_in_effect;

    std::deque<unsigned> m_open_variables;
    std::vector<unsigned> m_achievers;
    std::vector<int> m_layer;

    std::vector<bool> m_closed;
    Plan m_relaxed_plan;

    void verify();
    virtual SearchStatus step() override;
public:
    DelRaxSearch(const Options &opts);
    virtual void reset() override;
    virtual int calculate_plan_cost() const override;

    std::vector<int> &get_positive_values() { return m_positive_values; }
    std::vector<int> &get_reward() {return m_reward; }

    virtual void initialize() override;
    // template<typename Callback>
    // int evaluate_solution(const Callback &callback) const;
    static void add_options_to_parser(OptionParser &parser);
};

// template<typename Callback>
// int DelRaxSearch::evaluate_solution(const Callback &callback) const
// {
//     int result = 0;
//     for (unsigned var = 0; var < m_achieved.size(); var++) {
//         result += callback(std::pair<int, int>(var, m_positive_values[var]));
//     }
//     return result;
// }

}


#endif
