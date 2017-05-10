#ifndef SECOND_ORDER_SEARCH_DEPTH_FIRST_SEARCH_H
#define SECOND_ORDER_SEARCH_DEPTH_FIRST_SEARCH_H

#include "second_order_task_search.h"

#include "per_state_storage.h"
#include "../int_packer.h"

#include "../state_id.h"

#include <deque>
#include <vector>
#include <utility>
#include <unordered_map>

namespace second_order_search
{

class SuccessorPruningMethod;

class DepthFirstSearch : public SecondOrderTaskSearch
{
protected:
    // struct CallStackElement {
    //     StateID state_id;
    //     std::vector<const GlobalOperator *> aops;
    //     std::vector<bool> sleep;
    //     size_t i;
    //     IntPacker::Bin *counter;
    //     CallStackElement(const GlobalState &state);
    //     CallStackElement(const GlobalState &state,
    //                      const std::vector<bool> &sleep,
    //                      IntPacker::Bin *counter);
    // };
    size_t m_stat_expanded;
    size_t m_stat_updated;
    size_t m_stat_evaluated;

    size_t m_stat_last_printed;

    PerStateStorage<std::pair<int, int> > m_state_rewards;
    // std::deque<CallStackElement> m_stack;
    int m_max_reward;
    int m_cutoff;

    SuccessorPruningMethod *m_pruning_method;

    std::vector<const GlobalOperator *> m_current_path;
    std::unordered_map<StateID, std::vector<const GlobalOperator *> > m_paths;

    bool _insert_into_pareto_frontier(const std::pair<int, int> &val,
                                      const StateID &state_id);

    bool update_g_values(const GlobalState &state, int newg);
    void expand(const GlobalState &state,
                IntPacker::Bin *counter,
                std::vector<size_t> &sleep);

    virtual void initialize();
    virtual SearchStatus step() override;
    virtual void get_paths(const StateID &state,
                           std::vector<std::vector<const GlobalOperator *> > &paths) override;
    void print_statistic_line();
public:
    DepthFirstSearch(const Options &opts);
    // virtual void statistics() const override;
    static void add_options_to_parser(OptionParser &parser);
};

}

#endif
