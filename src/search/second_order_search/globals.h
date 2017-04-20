#ifndef SECOND_ORDER_TASK_GLOBALS_H
#define SECOND_ORDER_TASK_GLOBALS_H

#include "../global_operator.h"
#include "../successor_generator.h"
#include "../int_packer.h"
#include "../state_registry.h"
#include "../global_state.h"

#include <vector>
#include <string>

namespace second_order_search
{

void preprocess_second_order_task();

GlobalState g_outer_initial_state();

extern std::vector<int> g_outer_variable_domain;
extern std::vector<std::string> g_outer_variable_name;
extern std::vector<std::vector<std::string> > g_outer_fact_names;

extern std::vector<int> g_outer_initial_state_data;

extern std::vector<GlobalOperator> g_inner_operators;
extern std::vector<std::vector<GlobalCondition> >
g_inner_operator_outer_conditions;
extern std::vector<GlobalOperator> g_outer_operators;

extern SuccessorGenerator *g_outer_successor_generator;
extern SuccessorGenerator *g_outer_inner_successor_generator;

extern IntPacker *g_outer_state_packer;
extern StateRegistry *g_outer_state_registry;

}

#endif
