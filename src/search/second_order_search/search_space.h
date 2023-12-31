#ifndef SECOND_ORDER_SEARCH_SEARCH_SPACE_H
#define SECOND_ORDER_SEARCH_SEARCH_SPACE_H

#include "search_node_info.h"
#include "../state_id.h"
#include "../global_operator.h"

#include "../segmented_vector.h"

#include <vector>

namespace second_order_search
{

class SearchSpace;

class SearchNode
{
private:
    friend class SearchSpace;

    StateID state_id;
    SearchNodeInfo &info;
    SearchNode(const StateID &state_id,
               SearchNodeInfo &info);
public:
    StateID get_state_id() const;
    const SearchNodeInfo &get_info() const;

    bool is_new() const;
    bool is_open() const;
    bool is_closed() const;

    int get_g() const;
    int get_reward() const;

#ifdef COMPUTE_COMPLETE_PARETO_FRONTIER
    const std::vector<std::pair<const GlobalOperator *, StateID> > &get_parents()
    const;
    void add_parent(const GlobalOperator *op, const StateID &parent);
#else
    StateID get_parent_state_id() const;
    const GlobalOperator *get_parent_operator() const;
#endif

    void open_initial();
    void open(const StateID &parent, const GlobalOperator *op, int g);
    void close();

    void set_reward(int reward);

    IntPacker::Bin *&get_counter();
    const IntPacker::Bin *get_counter() const;

    IntPacker::Bin *&get_sleep();
    const IntPacker::Bin *get_sleep() const;
};

class SearchSpace
{
private:
    SegmentedVector<SearchNodeInfo> m_infos;
    void print_backtrace(const SearchNode &node,
                         std::vector<const GlobalOperator *> &labels,
                         size_t &counter);
    void backtrace(const SearchNode &node,
                   std::vector<const GlobalOperator *> &labels,
                   std::vector<std::vector<const GlobalOperator *> > &sequences);
public:
    SearchNode operator[](const StateID &state_id);
    SearchNode operator[](const GlobalState &state_id);
    void print_backtrace(const StateID &start, size_t &counter);
    void backtrace(const StateID &start,
                   std::vector<std::vector<const GlobalOperator *> > &sequences);
};

}


#endif
