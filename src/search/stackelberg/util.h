#ifndef STACKELBERG_UTIL_H
#define STACKELBERG_UTIL_H

#include <vector>
#include <string>

#include "../global_operator.h"
#include "../successor_generator.h"

//#ifndef LEADER_SEARCH_DEBUG
//#define LEADER_SEARCH_DEBUG 0
//#endif



namespace stackelberg {

    inline std::string ops_to_string(std::vector<const GlobalOperator *> &ops) {
        sort(ops.begin(), ops.end(), [] (const GlobalOperator * op1, const GlobalOperator * op2) {
            return op1->get_name() < op2->get_name();
        });
    std::string res = "";
    for (size_t i = 0; i < ops.size(); i++) {
        if(i > 0) {
            res += " ";
        }
        res += ops[i]->get_name();
    }
    return res;
}



/**
 * returns a SuccessorGeneratorSwitch based on the preconditions of the ops in pre_cond_ops and entailing the ops from ops vector in the leaves
 */
inline SuccessorGeneratorSwitch * create_successor_generator(
    const std::vector<int> &variable_domain,
    const std::vector<GlobalOperator> &pre_cond_ops, const std::vector<GlobalOperator> &ops)
    {
#ifdef LEADER_SEARCH_DEBUG
        std::cout << "Begin create_successor_generator..." << std::endl;
#endif

        int root_var_index = 0;

        auto root_node = new SuccessorGeneratorSwitch (root_var_index, variable_domain[root_var_index]);

        for (size_t op_no = 0; op_no < pre_cond_ops.size(); op_no++) {
            const auto & conditions = pre_cond_ops[op_no].get_preconditions();

            if (conditions.empty()) {
		// This op has no preconditions, add it immediately to the root node
		if (root_node->immediate_ops == nullptr) {
		    root_node->immediate_ops = new SuccessorGeneratorGenerate();
		}
		((SuccessorGeneratorGenerate *) root_node->immediate_ops)->add_op(&ops[op_no]);
		continue;
	    }

            SuccessorGeneratorSwitch *current_node = root_node;
            for (size_t cond_no = 0; cond_no < conditions.size(); cond_no++) {
		int var = conditions[cond_no].var;
		int val = conditions[cond_no].val;
		//cout << "Consider precond with var: " << var << ", val: " << val << endl;

		while (var != current_node->switch_var) {
		    if (current_node->default_generator == nullptr) {
			int next_var_index = current_node->switch_var + 1;
			current_node->default_generator = new SuccessorGeneratorSwitch(next_var_index,
										       variable_domain[next_var_index]);
		    }

		    current_node = (SuccessorGeneratorSwitch *) current_node->default_generator;
		}

		// Here: var == current_node->switch_var

		int next_var_index = current_node->switch_var + 1;
		if (next_var_index >= (int) variable_domain.size()) {
		    if (current_node->generator_for_value[val] == nullptr) {
			current_node->generator_for_value[val] = new SuccessorGeneratorGenerate();
		    }
		    ((SuccessorGeneratorGenerate *) current_node->generator_for_value[val])->add_op(
			&ops[op_no]);

		} else {
		    if (current_node->generator_for_value[val] == nullptr) {
			current_node->generator_for_value[val] = new SuccessorGeneratorSwitch(
			    next_var_index,
			    variable_domain[next_var_index]);
		    }

		    current_node = (SuccessorGeneratorSwitch *)
			current_node->generator_for_value[val];
		    if (cond_no == (conditions.size() - 1)) {
			// This was the last cond.
			if (current_node->immediate_ops == nullptr) {
			    current_node->immediate_ops = new SuccessorGeneratorGenerate();
			}
			((SuccessorGeneratorGenerate *) current_node->immediate_ops)->add_op(
			    &ops[op_no]);
		    }
		}
	    }
        }

	return root_node;
    }

}
#endif
