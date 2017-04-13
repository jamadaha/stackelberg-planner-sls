#ifndef SECOND_ORDER_SEARCH_SUCCESSOR_PRUNING_METHOD_H
#define SECOND_ORDER_SEARCH_SUCCESSOR_PRUNING_METHOD_H

class GlobalState;
class GlobalOperator;

#include <vector>

namespace second_order_search
{

class SuccessorPruningMethod
{
public:
    virtual void initialize() {}
    virtual void prune_successors(const GlobalState &state,
                                  const std::vector<const GlobalOperator *> &inner_plan,
                                  std::vector<const GlobalOperator *> &applicable_ops) = 0;
};

}

#endif
