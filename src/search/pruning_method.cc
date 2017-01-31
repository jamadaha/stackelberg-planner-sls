#include "pruning_method.h"

#include "global_operator.h"
#include "globals.h"
#include "plugin.h"

#include "utils/collections.h"

#include <algorithm>

using namespace std;

/* TODO: get_op_index belongs to a central place.
   We currently have copies of it in different parts of the code. */
static inline int get_op_index(const GlobalOperator *op) {
    int op_index = op - &*g_operators.begin();
    assert(op_index >= 0 && op_index < static_cast<int>(g_operators.size()));
    return op_index;
}


PruningMethod::PruningMethod()
     {
}

void PruningMethod::initialize() {

}


// TODO remove this overload once the search uses the task interface.
void PruningMethod::prune_operators(const GlobalState &state,
                                    vector<const GlobalOperator *> &global_ops) {
    /* Note that if the pruning method would use a different task than
       the search, we would have to convert the state before using it. */

    vector<int> op_ids;
    op_ids.reserve(global_ops.size());
    for (const GlobalOperator *global_op : global_ops) {
        op_ids.push_back(get_op_index(global_op));
    }

    prune_operators(state, op_ids);

    // If we pruned anything, translate the local operators back to global operators.
    if (op_ids.size() < global_ops.size()) {
        vector<const GlobalOperator *> pruned_ops;
        for (int op_id : op_ids) {
            pruned_ops.push_back(&g_operators[op_id]);
        }

        global_ops.swap(pruned_ops);
    }
}

