#ifndef SECOND_ORDER_SEARCH_SEARCH_NODE_INFO_H
#define SECOND_ORDER_SEARCH_SEARCH_NODE_INFO_H

#include "../state_id.h"

#ifdef COMPUTE_COMPLETE_PARETO_FRONTIER
#include <utility>
#include <vector>
#endif

class GlobalOperator;

namespace second_order_search
{

struct SearchNodeInfo {
    enum {NEW = 0, OPEN = 1, CLOSED = 2};
    unsigned status;
    unsigned g;
    unsigned r;

#ifdef COMPUTE_COMPLETE_PARETO_FRONTIER
    std::vector<std::pair<const GlobalOperator *, StateID> > parents;
#else
    StateID parent;
    const GlobalOperator *op;
#endif

#ifdef COMPUTE_COMPLETE_PARETO_FRONTIER
    SearchNodeInfo() :
        status(NEW),
        g(0),
        r(0)
    {}
#else
    SearchNodeInfo() :
        status(NEW),
        g(0),
        r(0),
        parent(StateID::no_state),
        op(NULL)
    {}
#endif
};

}

#endif
