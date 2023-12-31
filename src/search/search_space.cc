#include "search_space.h"

#include "global_operator.h"
#include "global_state.h"
#include "globals.h"

#include <cassert>
#include "search_node_info.h"

using namespace std;
using namespace __gnu_cxx;

int compute_remaining_budget(int budget, int cost) {
	if(budget == UNLTD_BUDGET) {
		return UNLTD_BUDGET;
	} else {
		return budget - cost;
	}
}

SearchNode::SearchNode(StateID state_id_, SearchNodeInfo &info_,
                       OperatorCost cost_type_, int _budget)
    : state_id(state_id_), info(info_), cost_type(cost_type_), budget(_budget) {
    assert(state_id != StateID::no_state);
}

GlobalState SearchNode::get_state() const {
    return g_state_registry->lookup_state(state_id);
}

bool SearchNode::is_open() const {
    return info.status == SearchNodeInfo::OPEN;
}

bool SearchNode::is_closed() const {
    return info.status == SearchNodeInfo::CLOSED;
}

bool SearchNode::is_dead_end() const {
    return info.status == SearchNodeInfo::DEAD_END;
}

bool SearchNode::is_new() const {
    return info.status == SearchNodeInfo::NEW;
}

int SearchNode::get_g() const {
    return info.g;
}

int SearchNode::get_real_g() const {
    return info.real_g;
}

int SearchNode::get_h() const {
    return info.h;
}

bool SearchNode::is_h_dirty() const {
    return info.h_is_dirty;
}

void SearchNode::set_h_dirty() {
    info.h_is_dirty = true;
}

void SearchNode::clear_h_dirty() {
    info.h_is_dirty = false;
}

void SearchNode::open_initial(int h) {
    assert(info.status == SearchNodeInfo::NEW);
    info.status = SearchNodeInfo::OPEN;
    info.g = 0;
    info.real_g = 0;
    info.h = h;
    info.parent_state_id = StateID::no_state;
    info.creating_operator = 0;
}

void SearchNode::open(int h, const SearchNode &parent_node,
                      const GlobalOperator *parent_op) {
    assert(info.status == SearchNodeInfo::NEW);
    info.status = SearchNodeInfo::OPEN;
    info.g = parent_node.info.g + get_adjusted_action_cost(*parent_op, cost_type);
    info.real_g = parent_node.info.real_g + parent_op->get_cost();
    info.h = h;
    info.parent_state_id = parent_node.get_state_id();
    info.creating_operator = parent_op;
}

void SearchNode::reopen(const SearchNode &parent_node,
                        const GlobalOperator *parent_op) {
    assert(info.status == SearchNodeInfo::OPEN ||
           info.status == SearchNodeInfo::CLOSED);

    // The latter possibility is for inconsistent heuristics, which
    // may require reopening closed nodes.
    info.status = SearchNodeInfo::OPEN;
    info.g = parent_node.info.g + get_adjusted_action_cost(*parent_op, cost_type);
    info.real_g = parent_node.info.real_g + parent_op->get_cost();
    info.parent_state_id = parent_node.get_state_id();
    info.creating_operator = parent_op;
}

// like reopen, except doesn't change status
void SearchNode::update_parent(const SearchNode &parent_node,
                               const GlobalOperator *parent_op) {
    assert(info.status == SearchNodeInfo::OPEN ||
           info.status == SearchNodeInfo::CLOSED);
    // The latter possibility is for inconsistent heuristics, which
    // may require reopening closed nodes.
    info.g = parent_node.info.g + get_adjusted_action_cost(*parent_op, cost_type);
    info.real_g = parent_node.info.real_g + parent_op->get_cost();
    info.parent_state_id = parent_node.get_state_id();
    info.creating_operator = parent_op;
}

void SearchNode::add_parent(const SearchNode &parent_node, const GlobalOperator *parent_op) {
	info.all_parent_state_ids.push_back(pair<StateID, int>(parent_node.get_state_id(), parent_node.get_budget()));
	info.all_parent_creating_operators.push_back(parent_op);
}

int SearchNode::increment_child_num() {
	info.num_non_dead_end_childs++;
	return info.num_non_dead_end_childs;
}

int SearchNode::decrement_child_num() {
	info.num_non_dead_end_childs--;
	return info.num_non_dead_end_childs;
}

int SearchNode::get_child_num() {
	return info.num_non_dead_end_childs;
}

void SearchNode::increase_h(int h) {
    assert(h >= info.h);
    info.h = h;
}

void SearchNode::close() {
    assert(info.status == SearchNodeInfo::OPEN);
    info.status = SearchNodeInfo::CLOSED;
}

void SearchNode::mark_as_dead_end() {
    info.status = SearchNodeInfo::DEAD_END;
}

void SearchNode::dump() const {
    cout << state_id << ": ";
    g_state_registry->lookup_state(state_id).dump_fdr();
    if (info.creating_operator) {
        cout << " created by " << info.creating_operator->get_name()
             << " from " << info.parent_state_id << endl;
    } else {
        cout << " no parent" << endl;
    }
}

SearchSpace::SearchSpace(OperatorCost cost_type_)
    : cost_type(cost_type_) {
}

SearchNode SearchSpace::get_node(const GlobalState &state, int budget) {
	BudgetSearchNodeInfo &budget_search_node_info = budget_search_node_infos[state];
	int index;

	if(budget_search_node_info.search_node_info_for_budget.find(budget) != budget_search_node_info.search_node_info_for_budget.end()) {
		index = budget_search_node_info.search_node_info_for_budget[budget];
	} else {
		all_search_node_infos.push_back(SearchNodeInfo());
		index = (int) all_search_node_infos.size() - 1;
		budget_search_node_info.search_node_info_for_budget[budget] = index;
	}

	return SearchNode(state.get_id(), all_search_node_infos[index], cost_type, budget);

	// return SearchNode(state.get_id(), search_node_infos[state], cost_type);
}

void SearchSpace::trace_path(const GlobalState &goal_state,
                             vector<const GlobalOperator *> &path, int current_budget) {
    GlobalState current_state = goal_state;
    assert(path.empty());
    for (;;) {
    	SearchNode node = get_node(current_state, current_budget);
        const SearchNodeInfo &info = node.info;
        const GlobalOperator *op = info.creating_operator;
        if (op == 0) {
            assert(info.parent_state_id == StateID::no_state);
            break;
        }
        path.push_back(op);
        current_state = g_state_registry->lookup_state(info.parent_state_id);
        current_budget = current_budget == UNLTD_BUDGET? current_budget : current_budget + op->get_cost2();
    }
    reverse(path.begin(), path.end());
}

void SearchSpace::dump() const {
    for (PerStateInformation<SearchNodeInfo>::const_iterator it =
             search_node_infos.begin(g_state_registry);
         it != search_node_infos.end(g_state_registry); ++it) {
        StateID id = *it;
        GlobalState s = g_state_registry->lookup_state(id);
        const SearchNodeInfo &node_info = search_node_infos[s];
        cout << id << ": ";
        s.dump_fdr();
        if (node_info.creating_operator && node_info.parent_state_id != StateID::no_state) {
            cout << " created by " << node_info.creating_operator->get_name()
                 << " from " << node_info.parent_state_id << endl;
        } else {
            cout << "has no parent" << endl;
        }
    }
}

void SearchSpace::statistics() const {
    cout << "Number of registered states: " << g_state_registry->size() << endl;
}
