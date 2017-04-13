#ifndef SECOND_ORDER_SEARCH_SUCCESSOR_PRUNING_METHOD_H
#define SECOND_ORDER_SEARCH_SUCCESSOR_PRUNING_METHOD_H

class GlobalState;
class GlobalOperator;

#include "search_space.h"

#include <vector>

namespace second_order_search
{

class SuccessorPruningMethod
{
protected:
    SearchSpace &m_search_space;
public:
    SuccessorPruningMethod(SearchSpace &search_space);
    virtual void initialize() {}
    virtual void prune_successors(const GlobalState &state,
                                  const std::vector<const GlobalOperator *> &inner_plan,
                                  std::vector<const GlobalOperator *> &applicable_ops) = 0;
};

}

#endif
