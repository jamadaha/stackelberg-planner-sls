#ifndef SECOND_ORDER_SEARCH_SEARCH_NODE_INFO_H
#define SECOND_ORDER_SEARCH_SEARCH_NODE_INFO_H

#include "../state_id.h"

class GlobalOperator;

namespace second_order_search
{

struct SearchNodeInfo {
    enum {NEW = 0, OPEN = 1, CLOSED = 2};
    unsigned status;
    unsigned g;
    unsigned r;

    StateID parent;
    const GlobalOperator *op;

    SearchNodeInfo() :
        status(NEW),
        g(0),
        r(0),
        parent(StateID::no_state),
        op(NULL)
    {}
};

}

#endif
