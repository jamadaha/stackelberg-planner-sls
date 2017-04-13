#ifndef SECOND_ORDER_SEARCH_SEARCH_SPACE_H
#define SECOND_ORDER_SEARCH_SEARCH_SPACE_H

#include "search_node_info.h"
#include "../state_id.h"
#include "../global_operator.h"

#include "../segmented_vector.h"

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

    StateID get_parent() const;
    const GlobalOperator *get_parent_operator() const;

    void open_initial();
    void open(const StateID &parent, const GlobalOperator *op, int g);
    void close();

    void set_reward(int reward);
};

class SearchSpace
{
private:
    SegmentedVector<SearchNodeInfo> m_infos;
public:
    SearchNode operator[](const StateID &state_id);
    SearchNode operator[](const GlobalState &state_id);
};

}


#endif
