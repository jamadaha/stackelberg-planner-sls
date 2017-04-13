/* NOTE: The following implementation is tailored to the internet mitigation
 * analysis scenario, and might not work in more general tasks!!! **/

#ifndef SECOND_ORDER_SEARCH_STRONG_STUBBORN_SET_H
#define SECOND_ORDER_SEARCH_STRONG_STUBBORN_SET_H

#include "successor_pruning_method.h"

#include <vector>

namespace second_order_search
{

class StrongStubbornSet : public SuccessorPruningMethod
{
    std::vector<std::vector<unsigned> > m_operator_negated_by;
    std::vector<bool> m_is_applicable;
public:
    virtual void initialize();
    virtual void prune_successors(const GlobalState &state,
                                  const std::vector<const GlobalOperator *> &inner_plan,
                                  std::vector<const GlobalOperator *> &aops) override;
};

}

#endif
