#ifndef SEARCH_SPACE_H
#define SEARCH_SPACE_H

#include "global_state.h"
#include "operator_cost.h"
#include "per_state_information.h"
#include "search_node_info.h"
#include <unordered_map>

#include <vector>

class GlobalOperator;
class GlobalState;


int compute_remaining_budget(int budget, int cost);

class SearchNode {
	friend class SearchSpace;

    StateID state_id;
    SearchNodeInfo &info;
    OperatorCost cost_type;
    int budget;
public:
    SearchNode(StateID state_id_, SearchNodeInfo &info_,
               OperatorCost cost_type_, int budget = UNLTD_BUDGET);

    StateID get_state_id() const {
        return state_id;
    }

    int get_budget() const {
    	return budget;
    }


    GlobalState get_state() const;

    bool is_new() const;
    bool is_open() const;
    bool is_closed() const;
    bool is_dead_end() const;

    bool is_h_dirty() const;
    void set_h_dirty();
    void clear_h_dirty();
    int get_g() const;
    int get_real_g() const;
    int get_h() const;

    void open_initial(int h);
    void open(int h, const SearchNode &parent_node,
              const GlobalOperator *parent_op);
    void reopen(const SearchNode &parent_node,
                const GlobalOperator *parent_op);
    void update_parent(const SearchNode &parent_node,
                       const GlobalOperator *parent_op);
    void add_parent(const SearchNode &parent_node, const GlobalOperator *parent_op);
    const std::vector<std::pair<StateID, int>> &get_all_parent_state_ids() const {return info.all_parent_state_ids; }
    const std::vector<const GlobalOperator*> &get_all_parent_creating_operators() const {return info.all_parent_creating_operators; }
    void increase_h(int h);
    void close();
    void mark_as_dead_end();
    int increment_child_num();
    int decrement_child_num();
    int get_child_num();

    void dump() const;
};

struct BudgetSearchNodeInfo {
	std::unordered_map<int, int> search_node_info_for_budget;

	BudgetSearchNodeInfo() {}
};

class SearchSpace {
    PerStateInformation<SearchNodeInfo> search_node_infos;
    PerStateInformation<BudgetSearchNodeInfo> budget_search_node_infos;
    SegmentedVector<SearchNodeInfo> all_search_node_infos;

    OperatorCost cost_type;
public:
    SearchSpace(OperatorCost cost_type_);
    SearchNode get_node(const GlobalState &state, int budget = UNLTD_BUDGET);
    void trace_path(const GlobalState &goal_state,
                    std::vector<const GlobalOperator *> &path, int budget = UNLTD_BUDGET);
    void reset() {
    	search_node_infos.remove_state_registry(g_state_registry);
    	budget_search_node_infos.remove_state_registry(g_state_registry);
    	all_search_node_infos.resize(0);
    }

    void dump() const;
    void statistics() const;
};

#endif
