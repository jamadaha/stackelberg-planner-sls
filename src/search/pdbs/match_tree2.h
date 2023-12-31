#ifndef PDBS_MATCH_TREE2_H
#define PDBS_MATCH_TREE2_H

#include "types.h"

#include <cstddef>
#include <vector>

namespace pdbs
{
class AbstractOperator;

// Successor Generator for abstract operators.
class MatchTree2
{
    struct Node;
    Pattern pattern;
    std::vector<size_t> hash_multipliers;
    Node *root;
    void insert_recursive(const AbstractOperator &op,
                          const std::vector<std::pair<int, int>> &preconditions,
                          int pre_index,
                          Node **edge_from_parent);
    void get_applicable_operators_recursive(
        Node *node, const size_t state_index,
        std::vector<const AbstractOperator *> &applicable_operators) const;
    void dump_recursive(Node *node) const;
public:
    // Initialize an empty match tree.
    MatchTree2(const Pattern &pattern,
              const std::vector<size_t> &hash_multipliers);
    ~MatchTree2();
    /* Insert an abstract operator into the match tree, creating or
       enlarging it. */
    void insert(const AbstractOperator &op,
                const std::vector<std::pair<int, int>> &preconditions);

    /*
      Extracts all applicable abstract operators for the abstract state given
      by state_index (the index is converted back to variable/values pairs).
    */
    void get_applicable_operators(
        size_t state_index,
        std::vector<const AbstractOperator *> &applicable_operators) const;
    void dump() const;
};
}

#endif
