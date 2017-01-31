#include "stubborn_sets_simple.h"

#include "../option_parser.h"
#include "../plugin.h"

using namespace std;

namespace stubborn_sets_simple {
void StubbornSetsSimple::initialize() {
    StubbornSets::initialize();
    compute_interference_relation();
    cout << "pruning method: stubborn sets simple" << endl;
}


void StubbornSetsSimple::compute_interference_relation() {
    interference_relation.resize(num_operators);

    /*
       TODO: as interference is symmetric, we only need to compute the
       relation for operators (o1, o2) with (o1 < o2) and add a lookup
       method that looks up (i, j) if i < j and (j, i) otherwise.
    */
    for (int op1_no = 0; op1_no < num_operators; ++op1_no) {
        vector<int> &interfere_op1 = interference_relation[op1_no];
        for (int op2_no = 0; op2_no < num_operators; ++op2_no) {
            if (op1_no != op2_no && interfere(op1_no, op2_no)) {
                interfere_op1.push_back(op2_no);
            }
        }
    }
}

// Add all operators that achieve the fact (var, value) to stubborn set.
void StubbornSetsSimple::add_necessary_enabling_set(const pair <int,int> &fact) {
    for (int op_no : achievers[fact.first][fact.second]) {
        mark_as_stubborn(op_no);
    }
}

// Add all operators that interfere with op.
void StubbornSetsSimple::add_interfering(int op_no) {
    for (int interferer_no : interference_relation[op_no]) {
        mark_as_stubborn(interferer_no);
    }
}

void StubbornSetsSimple::initialize_stubborn_set(const GlobalState &state) {
    // Add a necessary enabling set for an unsatisfied goal.
	pair <int,int> unsatisfied_goal = find_unsatisfied_goal(state);
    assert(unsatisfied_goal != NO_FACT);
    add_necessary_enabling_set(unsatisfied_goal);
}

void StubbornSetsSimple::handle_stubborn_operator(const GlobalState &state,
                                                  int op_no) {
	pair <int,int> unsatisfied_precondition = find_unsatisfied_precondition(op_no, state);
    if (unsatisfied_precondition == NO_FACT) {
        /* no unsatisfied precondition found
           => operator is applicable
           => add all interfering operators */
        add_interfering(op_no);
    } else {
        /* unsatisfied precondition found
           => add a necessary enabling set for it */
        add_necessary_enabling_set(unsatisfied_precondition);
    }
}

PruningMethod* _parse(OptionParser &parser) {
    parser.document_synopsis(
        "Stubborn sets simple",
        "Stubborn sets represent a state pruning method which computes a subset "
        "of applicable operators in each state such that completeness and "
        "optimality of the overall search is preserved. As stubborn sets rely "
        "on several design choices, there are different variants thereof. "
        "The variant 'StubbornSetsSimple' resolves the design choices in a "
        "straight-forward way.");

    if (parser.dry_run()) {
        return nullptr;
    }

    return new StubbornSetsSimple();
}

Plugin<PruningMethod> _plugin("stubborn_sets_simple", _parse);
}
