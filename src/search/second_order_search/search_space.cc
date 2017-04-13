
#include "search_space.h"

namespace second_order_search
{

SearchNode::SearchNode(const StateID &state_id,
                       SearchNodeInfo &info) :
    state_id(state_id),
    info(info)
{
}

StateID SearchNode::get_state_id() const
{
    return state_id;
}

const SearchNodeInfo &SearchNode::get_info() const
{
    return info;
}

bool SearchNode::is_new() const
{
    return    info.status == SearchNodeInfo::NEW;
}

bool SearchNode::is_open() const
{
    return info.status == SearchNodeInfo::OPEN;
}

bool SearchNode::is_closed() const
{
    return info.status == SearchNodeInfo::CLOSED;
}

int SearchNode::get_g() const
{
    return info.g;
}

int SearchNode::get_reward() const
{
    return info.r;
}

StateID SearchNode::get_parent() const
{
    return info.parent;
}

const GlobalOperator *SearchNode::get_parent_operator() const
{
    return info.op;
}

void SearchNode::open_initial()
{
    info.status = SearchNodeInfo::OPEN;
    info.g = 0;
    info.op = NULL;
}

void SearchNode::open(const StateID &parent, const GlobalOperator *op, int g)
{
    info.status = SearchNodeInfo::OPEN;
    info.g = g;
    info.parent = parent;
    info.op = op;
}

void SearchNode::close()
{
    info.status = SearchNodeInfo::CLOSED;
}

void SearchNode::set_reward(int r)
{
    info.r = r;
}

SearchNode SearchSpace::operator[](const StateID &state_id)
{
    size_t i = state_id.hash();
    if (i >= m_infos.size()) {
        m_infos.resize(i + 1);
    }
    return SearchNode(state_id, m_infos[i]);
}

SearchNode SearchSpace::operator[](const GlobalState &state)
{
    return this->operator[](state.get_id());
}

}
