/* NOTE: The following implementation is tailored to the internet mitigation
 * analysis scenario, and might not work in more general tasks!!! **/

#ifndef SECOND_ORDER_BEST_FIRST_SEARCH_H
#define SECOND_ORDER_BEST_FIRST_SEARCH_H

#include "second_order_task_search.h"

#include "open_list.h"
#include "search_space.h"
#include "../state_id.h"

#include "successor_pruning_method.h"

#include "../timer.h"

#include <map>
#include <vector>
#include <functional>

class Options;
class OptionParser;
class GlobalOperator;

namespace second_order_search
{

class BestFirstSearch : public SecondOrderTaskSearch
{
    typedef std::map<int, std::pair<int, std::vector<StateID> >, std::greater<int> >
    ParetoFrontier;

    size_t m_stat_last_printed_states;
    size_t m_stat_last_printed_pareto;

    size_t m_stat_open;
    size_t m_stat_expanded;
    size_t m_stat_generated;
    size_t m_stat_evaluated;
    size_t m_stat_pruned_successors;

    int m_stat_current_g;

    void force_print_statistic_line() const;
    void print_statistic_line();

protected:
    const bool c_silent;
    const bool c_precompute_max_reward;
    const bool c_lazy_reward_computation;

    int m_g_limit;
    int m_max_reward;

    SearchSpace m_search_space;
    OpenList<StateID> *m_open_list;
    ParetoFrontier m_pareto_frontier;

    SuccessorPruningMethod *m_pruning_method;

    std::vector <const GlobalOperator *> m_applicable_operators;

    void set_reward(const SearchNode &parent,
                    const GlobalOperator &op,
                    SearchNode &node);
    void insert_into_pareto_frontier(const SearchNode &node);

    virtual void initialize() override;
    virtual SearchStatus step() override;
public:
    BestFirstSearch(const Options &opts);
    virtual void save_plan_if_necessary() override;
    static void add_options_to_parser(OptionParser &parser);
};

}

#endif
