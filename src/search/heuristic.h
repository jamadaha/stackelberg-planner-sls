#ifndef HEURISTIC_H
#define HEURISTIC_H

#include "scalar_evaluator.h"
#include "operator_cost.h"
#include "globals.h"
#include "search_space.h"
#include "open_lists/open_list.h"

#include <map>
#include <set>
#include <string>
#include <vector>
#include <limits>

class GlobalOperator;
class GlobalState;
class OptionParser;
class Options;

struct AttackSearchInfo;
struct AttackSearchSpace;


class Heuristic : public ScalarEvaluator {
    enum {NOT_INITIALIZED = -2};
    int heuristic;
    int evaluator_value; // usually equal to heuristic but can be different
    // if set with set_evaluator_value which is done if we use precalculated
    // estimates, eg. when re-opening a search node

    std::vector<const GlobalOperator *> preferred_operators;
protected:
    OperatorCost cost_type;
    enum {DEAD_END = -1};

    bool needs_buget = false;

    virtual void initialize() {}
    virtual int compute_heuristic(const GlobalState &state) = 0;
    virtual int compute_heuristic(const GlobalState&, int) {return 0; }

    // Usage note: It's OK to set the same operator as preferred
    // multiple times -- it will still only appear in the list of
    // preferred operators for this heuristic once.
    void set_preferred(const GlobalOperator *op);
    int get_adjusted_cost(const GlobalOperator &op) const;
public:
    Heuristic(const Options &options);
    virtual ~Heuristic();

    void evaluate(const GlobalState &state, int budget = UNLTD_BUDGET, int lower_bound = 0);
    bool is_dead_end() const;
    int get_heuristic();
    // changed to virtual, so HeuristicProxy can delegate this:
    virtual void get_preferred_operators(std::vector<const GlobalOperator *> &result);
    virtual bool dead_ends_are_reliable() const {return true; }
    virtual bool reach_state(const GlobalState &parent_state, const GlobalOperator &op,
                             const GlobalState &state);

    // for abstract parent ScalarEvaluator
    int get_value() const;
    void evaluate(int g, bool preferred);
    bool dead_end_is_reliable() const;
    void set_evaluator_value(int val);
    void get_involved_heuristics(std::set<Heuristic *> &hset) {hset.insert(this); }
    OperatorCost get_cost_type() const {return cost_type; }

    static void add_options_to_parser(OptionParser &parser);
    static Options default_options();

    virtual void reinitialize(AttackSearchSpace* , SearchSpace* , OpenList<std::pair<StateID, int>>* , GlobalState , int ) { }
    virtual void set_curr_attack_search_space (AttackSearchSpace* ) { }
    virtual AttackSearchSpace* get_curr_attack_search_space () {return NULL; }
    virtual void reset () {std::cout << "This heuristic has not implemented reset()-method" << std::endl; exit(EXIT_UNSUPPORTED); }
};

#endif
