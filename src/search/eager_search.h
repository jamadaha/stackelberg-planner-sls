#ifndef EAGER_SEARCH_H
#define EAGER_SEARCH_H

#include <vector>

#include "evaluator.h"
#include "global_state.h"
#include "search_engine.h"
#include "search_progress.h"
#include "search_space.h"
#include "utils/timer.h"
#include "pruning_method.h"

#include "open_lists/open_list.h"

class GlobalOperator;
class Heuristic;
class Options;
class ScalarEvaluator;

class EagerSearch : public SearchEngine {
    // Search Behavior parameters
    bool reopen_closed_nodes; // whether to reopen closed nodes upon finding lower g paths
    bool do_pathmax; // whether to use pathmax correction
    bool use_multi_path_dependence;

    OpenList<std::pair<StateID, int>> *open_list;
    ScalarEvaluator *f_evaluator;

    PruningMethod* pruning_method;

    bool pruning_already_initialized = false;
    const bool use_heuristics_for_bound_pruning;
    
protected:
    SearchStatus step();
    std::pair<SearchNode, bool> fetch_next_node();
    void update_jump_statistic(const SearchNode &node);
    void print_heuristic_values(const std::vector<int> &values) const;
    void reward_progress();

    std::vector<Heuristic *> heuristics;
    std::vector<Heuristic *> preferred_operator_heuristics;
    std::vector<Heuristic *> estimate_heuristics;
    // TODO: in the long term this
    // should disappear into the open list

    virtual void initialize();

public:
    EagerSearch(const Options &opts);
    void statistics() const;
    virtual void reset() override;
    OpenList<std::pair<StateID, int>>* get_open_list() {return open_list; }
    void dump_search_space();
};

#endif
