#include "globals.h"

#include "axioms.h"
#include "causal_graph.h"
#include "domain_transition_graph.h"
#include "global_operator.h"
#include "global_state.h"
#include "heuristic.h"
#include "int_packer.h"
#include "rng.h"
#include "state_registry.h"
#include "successor_generator.h"
#include "utils/timer.h"
#include "utilities.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <limits>
#include <set>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_set>
using namespace std;

#include <ext/hash_map>
using namespace __gnu_cxx;

static const int PRE_FILE_VERSION = 3;


bool test_goal(const GlobalState &state) {
    for (size_t i = 0; i < g_goal.size(); ++i) {
        if (state[g_goal[i].first] != g_goal[i].second) {
            return false;
        }
    }
    return true;
}

bool test_goal(const std::vector<int> &state) {
    for (size_t i = 0; i < g_goal.size(); ++i) {
        if (state[g_goal[i].first] != g_goal[i].second) {
            return false;
        }
    }
    return true;
}

int calculate_plan_cost(const vector<const GlobalOperator *> &plan) {
    // TODO: Refactor: this is only used by save_plan (see below)
    //       and the SearchEngine classes and hence should maybe
    //       be moved into the SearchEngine (along with save_plan).
    int plan_cost = 0;
    for (size_t i = 0; i < plan.size(); ++i) {
        plan_cost += plan[i]->get_cost();
    }
    return plan_cost;
}

int calculate_fix_actions_plan_cost(const vector<const GlobalOperator *> &plan) {
    int plan_cost = 0;
    /* We removed the cost2 initial costs from fix actions
    unordered_set<int> scheme_ids;
    for (size_t i = 0; i < plan.size(); ++i) {
        plan_cost += plan[i]->get_cost();
        int scheme_id = plan[i]->get_scheme_id();
        if(scheme_ids.find(scheme_id) == scheme_ids.end()) {
        	scheme_ids.insert(scheme_id);
        	plan_cost += plan[i]->get_cost2();
        }
    }
    */
    for (size_t i = 0; i < plan.size(); ++i) {
        plan_cost += plan[i]->get_cost();
    }
    return plan_cost;
}

void save_plan(const vector<const GlobalOperator *> &plan, int) {
    // TODO: Refactor: this is only used by the SearchEngine classes
    //       and hence should maybe be moved into the SearchEngine.
 /*   ofstream outfile;
    if (iter == 0) {
        outfile.open(g_plan_filename.c_str(), ios::out);
    } else {
        ostringstream out;
        out << g_plan_filename << "." << iter;
        outfile.open(out.str().c_str(), ios::out);
    }
    for (size_t i = 0; i < plan.size(); ++i) {
        cout << plan[i]->get_name() << " (" << plan[i]->get_cost() << ")" << endl;
        outfile << "(" << plan[i]->get_name() << ")" << endl;
    }
    int plan_cost = calculate_plan_cost(plan);
    outfile << "; cost = " << plan_cost << " ("
            << (is_unit_cost() ? "unit-cost" : "general-cost") << ")" << endl;
    outfile.close();
    cout << "Plan length: " << plan.size() << " step(s)." << endl;
    cout << "Plan cost: " << plan_cost << endl;
*/

/*#ifndef NDEBUG
    for (size_t i = 0; i < plan.size(); ++i) {
        cout << plan[i]->get_name() << " (" << plan[i]->get_cost() << ")" << endl;
    }
#endif */
    g_plan.clear();
    g_plan.insert(g_plan.begin(), plan.begin(), plan.end());
}

bool peek_magic(istream &in, string magic) {
    string word;
    in >> word;
    bool result = (word == magic);
    for (int i = word.size() - 1; i >= 0; --i)
        in.putback(word[i]);
    return result;
}

void check_magic(istream &in, string magic) {
    string word;
    in >> word;
    if (word != magic) {
        cout << "Failed to match magic word '" << magic << "'." << endl;
        cout << "Got '" << word << "'." << endl;
        if (magic == "begin_version") {
            cerr << "Possible cause: you are running the planner "
                 << "on a preprocessor file from " << endl
                 << "an older version." << endl;
        }
        exit_with(EXIT_INPUT_ERROR);
    }
}

void read_and_verify_version(istream &in) {
    int version;
    check_magic(in, "begin_version");
    in >> version;
    check_magic(in, "end_version");
    if (version != PRE_FILE_VERSION) {
        cerr << "Expected preprocessor file version " << PRE_FILE_VERSION
             << ", got " << version << "." << endl;
        cerr << "Exiting." << endl;
        exit_with(EXIT_INPUT_ERROR);
    }
}

void read_metric(istream &in) {
    check_magic(in, "begin_metric");
    in >> g_use_metric;
    check_magic(in, "end_metric");
}

void read_variables(istream &in) {
    int count;
    in >> count;
    for (int i = 0; i < count; ++i) {
        check_magic(in, "begin_variable");
        string name;
        in >> name;
        g_variable_name.push_back(name);
        int layer;
        in >> layer;
        g_axiom_layers.push_back(layer);
        int range;
        in >> range;
        g_variable_domain.push_back(range);
        in >> ws;
        vector<string> fact_names(range);
        for (size_t j = 0; j < fact_names.size(); ++j)
            getline(in, fact_names[j]);
        g_fact_names.push_back(fact_names);
        check_magic(in, "end_variable");
        //Alvaro, Vidal: Important set id_facts
        g_id_first_fact.push_back(g_num_facts);
        g_num_facts += range;

    }
}

void read_mutexes(istream &in) {
    g_inconsistent_facts.resize(g_num_facts*g_num_facts, false);

    int num_mutex_groups;
    in >> num_mutex_groups;

    /* NOTE: Mutex groups can overlap, in which case the same mutex
       should not be represented multiple times. The current
       representation takes care of that automatically by using sets.
       If we ever change this representation, this is something to be
       aware of. */

    for (int i = 0; i < num_mutex_groups; ++i) {
        MutexGroup mg = MutexGroup(in);
        g_mutex_groups.push_back(mg);
  
        const auto & invariant_group = mg.getFacts();
        for (size_t j = 0; j < invariant_group.size(); ++j) {
            const auto &fact1 = invariant_group[j];
            for (size_t k = 0; k < invariant_group.size(); ++k) {
                const auto &fact2 = invariant_group[k];
                set_mutex(fact1.get_pair(), fact2.get_pair());
            }
        }
    }
}

void read_goal(istream &in) {
    check_magic(in, "begin_goal");
    int count;
    in >> count;
    if (count < 1) {
        cerr << "Task has no goal condition!" << endl;
        exit_with(EXIT_INPUT_ERROR);
    }
    for (int i = 0; i < count; ++i) {
        int var, val;
        in >> var >> val;
        g_goal.push_back(make_pair(var, val));
    }
    check_magic(in, "end_goal");
}

void dump_goal() {
    cout << "Goal Conditions:" << endl;
    for (size_t i = 0; i < g_goal.size(); ++i)
        cout << "  " << g_variable_name[g_goal[i].first] << ": "
             << g_goal[i].second << endl;
}

void read_operators(istream &in) {
    int count;
    in >> count;
    for (int i = 0; i < count; ++i)
        g_operators.push_back(GlobalOperator(in, false));
}

void read_axioms(istream &in) {
    int count;
    in >> count;
    for (int i = 0; i < count; ++i)
        g_axioms.push_back(GlobalOperator(in, true));

    g_axiom_evaluator = new AxiomEvaluator;
}

void read_everything(istream &in) {
    cout << "reading input... [t=" << g_timer << "]" << endl;
    read_and_verify_version(in);
    read_metric(in);
    read_variables(in);
    read_mutexes(in);
    g_initial_state_data.resize(g_variable_domain.size());
    check_magic(in, "begin_state");
    for (size_t i = 0; i < g_variable_domain.size(); ++i) {
        in >> g_initial_state_data[i];
    }
    check_magic(in, "end_state");
    g_default_axiom_values = g_initial_state_data;

    read_goal(in);
    read_operators(in);
    read_axioms(in);
    check_magic(in, "begin_SG");
    g_successor_generator = read_successor_generator(in);
    check_magic(in, "end_SG");
    DomainTransitionGraph::read_all(in);
    check_magic(in, "begin_CG"); // ignore everything from here

    cout << "done reading input! [t=" << g_timer << "]" << endl;

    // NOTE: causal graph is computed from the problem specification,
    // so must be built after the problem has been read in.

    cout << "building causal graph..." << flush;
    g_causal_graph = new CausalGraph;
    cout << "done! [t=" << g_timer << "]" << endl;

    cout << "packing state variables..." << flush;
    assert(!g_variable_domain.empty());
    g_state_packer = new IntPacker(g_variable_domain);
    cout << "Variables: " << g_variable_domain.size() << endl;
    cout << "Bytes per state: "
         << g_state_packer->get_num_bins() *
    g_state_packer->get_bin_size_in_bytes() << endl;
    cout << "done! [t=" << g_timer << "]" << endl;

    // NOTE: state registry stores the sizes of the state, so must be
    // built after the problem has been read in.
    g_state_registry = new StateRegistry;

    cout << "done initalizing global data [t=" << g_timer << "]" << endl;
}

void dump_everything() {
    cout << "Use metric? " << g_use_metric << endl;
    cout << "Min Action Cost: " << g_min_action_cost << endl;
    cout << "Max Action Cost: " << g_max_action_cost << endl;
    // TODO: Dump the actual fact names.
    cout << "Variables (" << g_variable_name.size() << "):" << endl;
    for (size_t i = 0; i < g_variable_name.size(); ++i)
        cout << "  " << g_variable_name[i]
             << " (range " << g_variable_domain[i] << ")" << endl;
    GlobalState initial_state = g_initial_state();
    cout << "Initial State (PDDL):" << endl;
    initial_state.dump_pddl();
    cout << "Initial State (FDR):" << endl;
    initial_state.dump_fdr();
    dump_goal();
    /*
    cout << "Successor Generator:" << endl;
    g_successor_generator->dump();
    for(int i = 0; i < g_variable_domain.size(); ++i)
      g_transition_graphs[i]->dump();
    */
}

bool is_unit_cost() {
    return g_min_action_cost == 1 && g_max_action_cost == 1;
}

bool has_axioms() {
    return !g_axioms.empty();
}

void verify_no_axioms() {
    if (has_axioms()) {
        cerr << "Heuristic does not support axioms!" << endl << "Terminating."
             << endl;
        exit_with(EXIT_UNSUPPORTED);
    }
}

static int get_first_conditional_effects_op_id() {
    for (size_t i = 0; i < g_operators.size(); ++i) {
        const vector<GlobalEffect> &effects = g_operators[i].get_effects();
        for (size_t j = 0; j < effects.size(); ++j) {
            const vector<GlobalCondition> &cond = effects[j].conditions;
            if (!cond.empty())
                return i;
        }
    }
    return -1;
}

bool has_conditional_effects() {
    return get_first_conditional_effects_op_id() != -1;
}

void verify_no_conditional_effects() {
    int op_id = get_first_conditional_effects_op_id();
    if (op_id != -1) {
        cerr << "Heuristic does not support conditional effects "
             << "(operator " << g_operators[op_id].get_name() << ")" << endl
             << "Terminating." << endl;
        exit_with(EXIT_UNSUPPORTED);
    }
}

void verify_no_axioms_no_conditional_effects() {
    verify_no_axioms();
    verify_no_conditional_effects();
}

bool are_mutex(const pair<int, int> &a, const pair<int, int> &b) {
    if (a.second == -1 || b.second == -1) {
        return false;
    }
    if (a.first == b.first){ // same variable: mutex iff different value
        return a.second != b.second;
    }
  return g_inconsistent_facts[id_mutex(a, b)];
}

int id_mutex(const std::pair<int, int> & a, const std::pair<int, int> &b){
  int id_a = g_id_first_fact [a.first] + a.second;
  int id_b = g_id_first_fact [b.first] + b.second;
  if(id_a < id_b){
    return g_num_facts*id_a + id_b;
  }else{
    return g_num_facts*id_b + id_a;
  }
}

void set_mutex(const pair<int, int> & a, const pair<int, int> &b){
  g_inconsistent_facts[id_mutex(a, b)] = true;
}


const GlobalState &g_initial_state() {
    return g_state_registry->get_initial_state();
}

bool g_use_metric;
int g_min_action_cost = numeric_limits<int>::max();
int g_max_action_cost = 0;
vector<string> g_variable_name;
vector<int> g_variable_domain;
vector<vector<string> > g_fact_names;
vector<int> g_axiom_layers;
vector<int> g_default_axiom_values;
IntPacker *g_state_packer;
vector<int> g_initial_state_data;
vector<pair<int, int> > g_goal;
vector<GlobalOperator> g_operators;
vector<GlobalOperator> g_all_attack_operators;
vector<bool> g_attack_op_included;
vector<GlobalOperator> g_axioms;
AxiomEvaluator *g_axiom_evaluator;
SuccessorGenerator *g_successor_generator;
vector<DomainTransitionGraph *> g_transition_graphs;
CausalGraph *g_causal_graph;

vector<MutexGroup> g_mutex_groups; 
vector<bool> g_inconsistent_facts;
int g_num_facts;
vector<int> g_id_first_fact;
LegacyCausalGraph *g_legacy_causal_graph;

utils::Timer g_timer;
string g_plan_filename = "sas_plan";
RandomNumberGenerator g_rng(2011); // Use an arbitrary default seed.
StateRegistry *g_state_registry = 0;
vector<const GlobalOperator*> g_plan;

int g_initial_budget = UNLTD_BUDGET;
