#ifndef PRUNING_STUBBORN_SETS_H
#define PRUNING_STUBBORN_SETS_H

#include "../pruning_method.h"
#include "../global_operator.h"

namespace stubborn_sets {
inline std::pair <int,int> find_unsatisfied_condition(
    const std::vector<std::pair <int,int>> &conditions, const GlobalState &state);

class StubbornSets : public PruningMethod {
    long num_unpruned_successors_generated;
    long num_pruned_successors_generated;

    /* stubborn[op_no] is true iff the operator with operator index
       op_no is contained in the stubborn set */
    std::vector<bool> stubborn;

    /*
      stubborn_queue contains the operator indices of operators that
      have been marked as stubborn but have not yet been processed
      (i.e. more operators might need to be added to stubborn because
      of the operators in the queue).
    */
    std::vector<int> stubborn_queue;

    void compute_sorted_operators();
    void compute_achievers();

protected:
    /*
      We copy some parts of the task here, so we can avoid the more expensive
      access through the task interface during the search.
    */
    int num_operators;
    std::vector<std::vector<std::pair <int,int>>> sorted_op_preconditions;
    std::vector<std::vector<std::pair <int,int>>> sorted_op_effects;
    std::vector<std::pair <int,int>> sorted_goals;

    /* achievers[var][value] contains all operator indices of
       operators that achieve the fact (var, value). */
    std::vector<std::vector<std::vector<int>>> achievers;

    bool can_disable(int op1_no, int op2_no) const;
    bool can_conflict(int op1_no, int op2_no) const;

    /*
      Return the first unsatified goal pair,
      or FactPair::no_fact if there is none.

      Note that we use a sorted list of goals here intentionally.
      (See comment on find_unsatisfied_precondition.)
    */
    std::pair <int,int> find_unsatisfied_goal(const GlobalState &state) const {
        return find_unsatisfied_condition(sorted_goals, state);
    }

    /*
      Return the first unsatified precondition,
      or FactPair::no_fact if there is none.

      Note that we use a sorted list of preconditions here intentionally.
      The ICAPS paper "Efficient Stubborn Sets: Generalized Algorithms and
      Selection Strategies" (Wehrle and Helmert, 2014) contains a limited study
      of this (see section "Strategies for Choosing Unsatisfied Conditions" and
      especially subsection "Static Variable Orderings"). One of the outcomes
      was the sorted version ("static orders/FD" in Table 1 of the paper)
      is dramatically better than choosing preconditions and goals randomly
      every time ("dynamic orders/random" in Table 1).

      The code also intentionally uses the "causal graph order" of variables
      rather than an arbitrary variable order. (However, so far, there is no
      experimental evidence that this is a particularly good order.)
    */
    std::pair <int,int> find_unsatisfied_precondition(int op_no, const GlobalState &state) const {
        return find_unsatisfied_condition(sorted_op_preconditions[op_no], state);
    }

    // Returns true iff the operators was enqueued.
    // TODO: rename to enqueue_stubborn_operator?
    bool mark_as_stubborn(int op_no);
    virtual void initialize_stubborn_set(const GlobalState &state) = 0;
    virtual void handle_stubborn_operator(const GlobalState &state, int op_no) = 0;
public:
    virtual void initialize() override;

    /* TODO: move prune_operators, and also the statistics, to the
       base class to have only one method virtual, and to make the
       interface more obvious */
    virtual void prune_operators(const GlobalState &state,
                                 std::vector<int> &op_ids) override;
    virtual void print_statistics() const override;
};

// Return the first unsatified condition, or FactPair::no_fact if there is none.
inline std::pair <int,int> find_unsatisfied_condition(
    const std::vector<std::pair <int,int>> &conditions, const GlobalState &state) {
    for (const std::pair <int,int> &condition : conditions) {
        if (state[condition.first] != condition.second)
            return condition;
    }
    return std::NO_FACT;
}
}

#endif
