
#include "search_space.h"

#include <iostream>

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

#ifdef COMPUTE_COMPLETE_PARETO_FRONTIER
void SearchNode::add_parent(const GlobalOperator *op, const StateID &parent)
{
    info.parents.emplace_back(op, parent);
}

const std::vector<std::pair<const GlobalOperator *, StateID> > &
SearchNode::get_parents() const
{
    return info.parents;
}
#else
StateID SearchNode::get_parent_state_id() const
{
    return info.parent;
}

const GlobalOperator *SearchNode::get_parent_operator() const
{
    return info.op;
}
#endif

void SearchNode::open_initial()
{
    info.status = SearchNodeInfo::OPEN;
    info.g = 0;
}

void SearchNode::open(const StateID &parent, const GlobalOperator *op, int g)
{
    info.status = SearchNodeInfo::OPEN;
    info.g = g;
#ifdef COMPUTE_COMPLETE_PARETO_FRONTIER
    info.parents.clear();
    info.parents.emplace_back(op, parent);
#else
    info.parent = parent;
    info.op = op;
#endif
}

void SearchNode::close()
{
    info.status = SearchNodeInfo::CLOSED;
}

void SearchNode::set_reward(int r)
{
    info.r = r;
}

IntPacker::Bin *&SearchNode::get_counter()
{
    return info.counter;
}

const IntPacker::Bin *SearchNode::get_counter() const
{
    return info.counter;
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

void SearchSpace::print_backtrace(const SearchNode &node,
                                  std::vector<const GlobalOperator *> &labels,
                                  size_t &counter)
{
#ifdef COMPUTE_COMPLETE_PARETO_FRONTIER
    if (node.get_parents().empty()) {
#else
    if (node.get_parent_state_id() == StateID::no_state) {
#endif
        std::cout << "        " << "---sequence-" << counter << "---" << std::endl;
        if (labels.empty()) {
            std::cout << "            <empty-sequence>" << std::endl;
        } else {
            for (int i = labels.size() - 1; i >= 0; i--) {
                std::cout << "            " << labels[i]->get_name() << std::endl;
            }
        }
        counter++;
    } else {
#ifdef COMPUTE_COMPLETE_PARETO_FRONTIER
        for (auto &x : node.get_parents()) {
            labels.push_back(x.first);
            print_backtrace(this->operator[](x.second), labels, counter);
            labels.pop_back();
        }
#else
        labels.push_back(node.get_parent_operator());
        print_backtrace(this->operator[](node.get_parent_state_id()), labels, counter);
        labels.pop_back();
#endif
    }
}

void SearchSpace::print_backtrace(const StateID &start, size_t &counter)
{
    std::vector<const GlobalOperator *> labels;
    print_backtrace(this->operator[](start), labels, counter);
}

}
