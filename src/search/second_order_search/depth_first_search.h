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

#define STORE_POLICY_IN_INFO 0

namespace second_order_search
{

class SuccessorPruningMethod;

class DepthFirstSearch : public SecondOrderTaskSearch
{
protected:
    struct StateInfo {
        int r;
        int g;
#if STORE_POLICY_IN_INFO
        const GlobalOperator *p_op;
        StateID p;
#endif
        StateInfo(int r, int g
#if STORE_POLICY_IN_INFO
                  , const GlobalOperator *op, const StateID &s
#endif
                 )
            : r(r),
              g(g)
#if STORE_POLICY_IN_INFO
            , p_op(op),
              p(s)
#endif
        {}
    };
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
    size_t m_stat_generated;

    size_t m_stat_last_printed;

    PerStateStorage<StateInfo> m_state_rewards;
    // std::deque<CallStackElement> m_stack;
    int m_max_reward;
    int m_cutoff;

    SuccessorPruningMethod *m_pruning_method;

#if !STORE_POLICY_IN_INFO
    std::vector<const GlobalOperator *> m_current_path;
    std::unordered_map<StateID, std::vector<const GlobalOperator *> > m_paths;
#endif

    bool _insert_into_pareto_frontier(const StateInfo &val,
                                      const StateID &state_id);

    bool update_g_values(const GlobalState &state,
#if STORE_POLICY_IN_INFO
                         const StateID &p,
                         const GlobalOperator *op,
#endif
                         int newg);
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
    virtual void statistics() const override;
    static void add_options_to_parser(OptionParser &parser);
};

}

#endif
