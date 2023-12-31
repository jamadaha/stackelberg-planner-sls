#ifndef GLOBALS_H
#define GLOBALS_H

#include "utils/timer.h"
#include "mutex_group.h"
#include <iosfwd>
#include <string>
#include <vector>
#include <limits>

#define UNLTD_BUDGET std::numeric_limits<int>::max()

class Axiom;
class AxiomEvaluator;
class CausalGraph;
class DomainTransitionGraph;
class GlobalOperator;
class GlobalState;
class IntPacker;
class LegacyCausalGraph;
class RandomNumberGenerator;
class SuccessorGenerator;
class StateRegistry;

bool test_goal(const GlobalState &state);
bool test_goal(const std::vector<int> &state);
void save_plan(const std::vector<const GlobalOperator *> &plan, int iter);
int calculate_plan_cost(const std::vector<const GlobalOperator *> &plan);
int calculate_fix_actions_plan_cost(const std::vector<const GlobalOperator *>
                                    &plan);

void read_everything(std::istream &in);
void dump_everything();

bool is_unit_cost();
bool has_axioms();
void verify_no_axioms();
bool has_conditional_effects();
void verify_no_conditional_effects();
void verify_no_axioms_no_conditional_effects();

void check_magic(std::istream &in, std::string magic);

bool are_mutex(const std::pair<int, int> &a, const std::pair<int, int> &b);
void set_mutex(const std::pair<int, int> & a, const std::pair<int, int> &b);
int id_mutex(const std::pair<int, int> & a, const std::pair<int, int> &b);

//Alvaro: Substituted previous mutex data structures by two list of
//mutex groups (to iterate over invariants) and a vector of bools to
//implement are_mutex (with an auxiliary vector to get the fluent id)
//and the number of problem fluents
extern std::vector<MutexGroup> g_mutex_groups; 
extern std::vector<bool> g_inconsistent_facts;
extern int g_num_facts;
extern std::vector<int> g_id_first_fact;

extern bool g_use_metric;
extern int g_min_action_cost;
extern int g_max_action_cost;

// TODO: The following five belong into a new Variable class.
extern std::vector<std::string> g_variable_name;
extern std::vector<int> g_variable_domain;
extern std::vector<std::vector<std::string> > g_fact_names;
extern std::vector<int> g_axiom_layers;
extern std::vector<int> g_default_axiom_values;

extern IntPacker *g_state_packer;
// This vector holds the initial values *before* the axioms have been evaluated.
// Use the state registry to obtain the real initial state.
extern std::vector<int> g_initial_state_data;
// TODO The following function returns the initial state that is registered
//      in g_state_registry. This is only a short-term solution. In the
//      medium term, we should get rid of the global registry.
extern const GlobalState &g_initial_state();
extern std::vector<std::pair<int, int> > g_goal;

extern std::vector<GlobalOperator> g_operators;
extern std::vector<GlobalOperator> g_all_attack_operators;

/* FIXME Because REMOVED DIVIDING VARIABLES, this is not correctly initialized!! */
extern std::vector<bool> g_attack_op_included;


extern std::vector<GlobalOperator> g_axioms;
extern AxiomEvaluator *g_axiom_evaluator;
extern SuccessorGenerator *g_successor_generator;
extern std::vector<DomainTransitionGraph *> g_transition_graphs;
extern CausalGraph *g_causal_graph;
extern LegacyCausalGraph *g_legacy_causal_graph;
extern utils::Timer g_timer;
extern std::string g_plan_filename;
extern RandomNumberGenerator g_rng;
// Only one global object for now. Could later be changed to use one instance
// for each problem in this case the method GlobalState::get_id would also have to be
// changed.
extern StateRegistry *g_state_registry;
extern std::vector<const GlobalOperator *> g_plan;

extern int g_initial_budget;



#endif
