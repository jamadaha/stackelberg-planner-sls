#ifndef DEAD_END_PDB_HEURISTIC_H
#define DEAD_END_PDB_HEURISTIC_H

#include "types.h"

#include "../heuristic.h"
#include "../utils/countdown_timer.h"

namespace pdbs
{
class DeadEndTreeNode;

class DeadEndCollection
{
    int num_dead_ends;
    DeadEndTreeNode *root;
public:
    DeadEndCollection();
    ~DeadEndCollection();

    void add(const std::vector<std::pair<int, int> > &dead);

    bool recognizes(const std::vector<std::pair<int, int> > &partial_state) const;
    bool recognizes(const GlobalState &state) const;

    int size()
    {
        return num_dead_ends;
    }
};

class PDBDeadEndDetectionHeuristic : public Heuristic
{
    size_t max_dead_ends;
    bool add_pattern_dead_ends(const Pattern &pattern,
                               const GlobalState &initial_state,
                               const utils::CountdownTimer &timer);
    DeadEndCollection dead_end_collection;
protected:
    virtual int compute_heuristic(const GlobalState &state) override;
public:
    explicit PDBDeadEndDetectionHeuristic(const Options &opts);
    virtual ~PDBDeadEndDetectionHeuristic() = default;
    static void add_options_to_parser(OptionParser &parser);
};
}

#endif
