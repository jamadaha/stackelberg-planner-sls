#ifndef PRUNING_METHOD_H
#define PRUNING_METHOD_H


#include <memory>
#include <vector>

class AbstractTask;
class GlobalOperator;
class GlobalState;

#define NO_FACT pair<int, int> (-1, -1)

class PruningMethod {
protected:

public:
    PruningMethod();
    virtual ~PruningMethod() = default;

    virtual void initialize();

    /* This method must not be called for goal states. This can be checked
       with assertions in derived classes. */
    virtual void prune_operators(const GlobalState &state,
                                 std::vector<int> &op_ids) = 0;
    // TODO remove this overload once the search uses the task interface.
    virtual void prune_operators(const GlobalState &state,
                                 std::vector<const GlobalOperator *> &ops);
    virtual void print_statistics() const = 0;
};

#endif
