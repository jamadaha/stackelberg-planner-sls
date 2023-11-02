#ifndef SECOND_ORDER_SEARCH_DEPTH_FIRST_SEARCH_H
#define SECOND_ORDER_SEARCH_DEPTH_FIRST_SEARCH_H

#include "second_order_task_search.h"

#include "../int_packer.h"
#include "../state_id.h"

#include <vector>
#include <unordered_map>

namespace second_order_search
{

class SuccessorPruningMethod;

class DepthFirstSearch : public SecondOrderTaskSearch
{
protected:
    // **** statistics ****
    size_t m_stat_expanded;
    size_t m_stat_updated;
    size_t m_stat_evaluated;
    size_t m_stat_generated;

    size_t m_stat_last_printed;
    // **** end-statistics ****

    const bool c_relevance_pruning;

    SuccessorPruningMethod *m_pruning_method;

    int m_max_reward;
    int m_cutoff;

    std::vector<size_t> m_inner_reward_variables;
    std::vector<std::vector<bool> > m_outer_op_relevance;

    // std::vector<size_t> m_relevant_inner_variables; // ?????

    std::vector<const GlobalOperator *> m_current_path;
    std::unordered_map<StateID, std::vector<const GlobalOperator *> > m_paths;

    bool _insert_into_pareto_frontier(const int &rew,
                                      const int &g,
                                      const StateID &state_id);

    void expand(const GlobalState &state,
                IntPacker::Bin *counter,
                std::vector<size_t> &sleep,
                std::vector<size_t> relevant,
                int pr,
                int pg);

    virtual void initialize() override;
    virtual SearchStatus step() override;
    virtual void get_paths(const StateID &state,
                           std::vector<std::vector<const GlobalOperator *> > &paths) override;
    void print_statistic_line();
public:
    DepthFirstSearch(const Options &opts);
    virtual void statistics() const override;
    static void add_options_to_parser(OptionParser &parser);
};

}

#endif
