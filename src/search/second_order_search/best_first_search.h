/* NOTE: The following implementation is tailored to the internet mitigation
 * analysis scenario, and might not work in more general tasks!!! **/

#ifndef SECOND_ORDER_BEST_FIRST_SEARCH_H
#define SECOND_ORDER_BEST_FIRST_SEARCH_H

#include "../search_engine.h"

#include "open_list.h"
#include "search_space.h"
#include "../state_id.h"

#include "successor_pruning_method.h"

#include <map>
#include <vector>
#include <functional>

class Options;
class OptionParser;
class GlobalOperator;

namespace second_order_search
{

class SORBestFirstSearch : public SearchEngine
{
    typedef std::map<int, std::pair<int, std::vector<StateID> >, std::greater<int> >
    ParetoFrontier;

    size_t m_stat_last_printed_states;
    size_t m_stat_last_printed_pareto;

    size_t m_stat_open;
    size_t m_stat_expanded;
    size_t m_stat_generated;
    size_t m_stat_pruned_successors;

    void print_statistic_line();

protected:
    int m_g_limit;

    SearchSpace m_search_space;
    OpenList<StateID> *m_open_list;
    ParetoFrontier m_pareto_frontier;

    std::vector <const GlobalOperator *> m_applicable_operators;
    SearchEngine *m_inner_search;

    SuccessorPruningMethod *m_pruning_method;

    void insert_into_pareto_frontier(const SearchNode &node);

    virtual void initialize() override;
    virtual SearchStatus step() override;
public:
    SORBestFirstSearch(const Options &opts);
    virtual void save_plan_if_necessary() const;
    static void add_options_to_parser(OptionParser &parser);
};

}

#endif
