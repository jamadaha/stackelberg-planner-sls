#include "dead_end_pdb_heuristic.h"

#include "pattern_database.h"
#include "pattern_generator.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "../globals.h"
#include "../global_state.h"

#include <memory>
#include <algorithm>

using namespace std;


namespace pdbs
{
class DeadEndTreeNode
{
public:
    virtual ~DeadEndTreeNode() = default;
    virtual void add(const std::vector<std::pair<int, int> > &partial_state,
                     int index = 0) = 0;
    virtual bool contains(const std::vector<std::pair<int, int> > &partial_state,
                          int index = 0) = 0;
    virtual bool contains(const GlobalState &state) = 0;
};

class DeadEndTreeLeafNode : public DeadEndTreeNode
{
public:
    virtual void add(const std::vector<std::pair<int, int> > & /*partial_state*/,
                     int /*index*/) override
    {
        // No need to add, this node already recognizes the dead end.
    }

    virtual bool contains(const std::vector<std::pair<int, int> >
                          & /*partial_state*/,
                          int /*index*/) override
    {
        return true;
    }

    virtual bool contains(const GlobalState & /*state*/) override
    {
        return true;
    }
};

class DeadEndTreeSwitchNode : public DeadEndTreeNode
{
    int var_id;
    vector<DeadEndTreeNode *> value_successors;
    DeadEndTreeNode *ignore_successor;
public:
    DeadEndTreeSwitchNode(unsigned var)
        : var_id(var),
          value_successors(g_variable_domain[var], nullptr),
          ignore_successor(nullptr)
    {
    }

    virtual ~DeadEndTreeSwitchNode()
    {
        for (DeadEndTreeNode *child : value_successors) {
            delete child;
        }
        delete ignore_successor;
    }

    virtual void add(const std::vector<std::pair<int, int> > &partial_state,
                     int index = 0) override
    {
        const std::pair<int, int> &current_fact = partial_state[index];
        //assert(current_fact.first >= var_id);
        int current_var = current_fact.first;
        int current_value = current_fact.second;
        DeadEndTreeNode **successor;
        int next_index = index;
        if (var_id == current_var) {
            successor = &value_successors[current_value];
            ++next_index;
        } else {
            successor = &ignore_successor;
        }

        if (*successor) {
            (*successor)->add(partial_state, next_index);
        } else {
            if (next_index == static_cast<int>(partial_state.size())) {
                *successor = new DeadEndTreeLeafNode();
            } else {
                unsigned next_var = partial_state[next_index].first;
                *successor = new DeadEndTreeSwitchNode(next_var);
                (*successor)->add(partial_state, next_index);
            }
        }
    }

    virtual bool contains(const std::vector<std::pair<int, int> > &partial_state,
                          int index = 0) override
    {
        if (index == static_cast<int>(partial_state.size())) {
            return false;
        }
        int current_index = index;
        while (current_index < static_cast<int>(partial_state.size())
                && partial_state[current_index].first < var_id) {
            current_index++;
        }
        int next_index = index + partial_state[index].first == var_id;
        if (current_index < static_cast<int>(partial_state.size())
                && partial_state[current_index].first == var_id) {
            DeadEndTreeNode *value_successor =
                value_successors[partial_state[current_index].second];
            if (value_successor
                    && value_successor->contains(partial_state, current_index + 1)) {
                return true;
            }
        }
        if (ignore_successor && ignore_successor->contains(partial_state, next_index)) {
            return true;
        }
        return false;
    }

    virtual bool contains(const GlobalState &state) override
    {
        DeadEndTreeNode *value_successor = value_successors[state[var_id]];
        if (value_successor && value_successor->contains(state)) {
            return true;
        }
        if (ignore_successor && ignore_successor->contains(state)) {
            return true;
        }
        return false;
    }
};

DeadEndCollection::DeadEndCollection()
    : num_dead_ends(0),
      root(nullptr)
{
}

DeadEndCollection::~DeadEndCollection()
{
    delete root;
}

void DeadEndCollection::add(const std::vector<std::pair<int, int> > &dead)
{
    assert(!dead.empty());
    if (!root) {
        root = new DeadEndTreeSwitchNode(dead[0].first);
    }
    root->add(dead);
    ++num_dead_ends;
}

bool DeadEndCollection::recognizes(const std::vector<std::pair<int, int> >
                                   &partial_state)
const
{
    return root && root->contains(partial_state);
}

bool DeadEndCollection::recognizes(const GlobalState &state) const
{
    if (root) {
        return root->contains(state);
    }
    return false;
}

PDBDeadEndDetectionHeuristic::PDBDeadEndDetectionHeuristic(
    const Options &opts)
    : Heuristic(opts),
      max_dead_ends(opts.get<int>("max_dead_ends"))
{
    PatternCollectionGenerator *pattern_generator =
        opts.get<PatternCollectionGenerator *>("patterns");
    utils::CountdownTimer timer(opts.get<double>("max_time"));
    GlobalState initial_state = g_initial_state();
    pattern_generator->generate([&](const Pattern & pattern) {
        return add_pattern_dead_ends(pattern, initial_state, timer);
    });
    cout << "Found " << dead_end_collection.size() << " dead ends in " << timer <<
         endl;
}

bool PDBDeadEndDetectionHeuristic::add_pattern_dead_ends(
    const Pattern &pattern,
    const GlobalState &initial_state,
    const utils::CountdownTimer &timer)
{
    PatternDatabase pdb(pattern, false, true);
    for (const vector<std::pair<int, int> > &dead : pdb.get_dead_ends()) {
        if (!dead_end_collection.recognizes(dead)) {
            assert(std::is_sorted(dead.begin(), dead.end()));
            dead_end_collection.add(dead);
        }
    }
    bool memory_exhausted = ((int) max_dead_ends <= dead_end_collection.size());
    bool initial_state_recognized = pdb.get_value(initial_state) ==
                                    numeric_limits<int>::max();
    if (initial_state_recognized) {
        cout << "Initial state recognised as dead end by pattern " << pattern << endl;
    }
    return memory_exhausted || initial_state_recognized || timer.is_expired();
}

int PDBDeadEndDetectionHeuristic::compute_heuristic(const GlobalState &state)
{
    if (dead_end_collection.recognizes(state)) {
        return DEAD_END;
    } else {
        return 0;
    }
}

static Heuristic *_parse(OptionParser &parser)
{
    parser.add_option<PatternCollectionGenerator *>(
        "patterns",
        "pattern generation method",
        "ordered_systematic(-1)");

    parser.add_option<int>(
        "max_dead_ends",
        "maximal number of dead ends stored before starting the search",
        "-1");

    parser.add_option<double>(
        "max_time",
        "",
        "900");

    Heuristic::add_options_to_parser(parser);
    Options opts = parser.parse();
    if (parser.dry_run())
        return 0;
    else
        return new PDBDeadEndDetectionHeuristic(opts);
}
static Plugin<Heuristic> _plugin("deadpdbs", _parse);
}



