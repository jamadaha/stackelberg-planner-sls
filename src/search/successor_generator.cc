#include "successor_generator.h"

#include "global_operator.h"
#include "global_state.h"
#include "globals.h"
#include "utilities.h"

#include <cstdlib>
#include <iostream>
#include <vector>
#include <cassert>

using namespace std;


SuccessorGeneratorSwitch::SuccessorGeneratorSwitch(istream &in)
{
    in >> switch_var;
    var_range = g_variable_domain[switch_var];
    immediate_ops = read_successor_generator(in);
    for (int i = 0; i < var_range; ++i) {
        generator_for_value.push_back(read_successor_generator(in));
    }
    default_generator = read_successor_generator(in);
}

SuccessorGeneratorSwitch::SuccessorGeneratorSwitch(int _switch_var,
        int _var_range)
    : switch_var(_switch_var),
      var_range(_var_range)
{
    generator_for_value.assign(_var_range, NULL);
    immediate_ops = NULL;
    default_generator = NULL;
}

SuccessorGeneratorSwitch::~SuccessorGeneratorSwitch()
{
    if (immediate_ops != NULL) {
        delete immediate_ops;
    }

    for (size_t generator_no = 0; generator_no < generator_for_value.size();
            generator_no++) {
        delete generator_for_value[generator_no];
    }

    if (default_generator != NULL) {
        delete default_generator;
    }
}

void SuccessorGeneratorSwitch::generate_applicable_ops(
    const GlobalState &curr, vector<const GlobalOperator *> &ops)
{
    if (immediate_ops != NULL) {
        immediate_ops->generate_applicable_ops(curr, ops);
    }
    if (generator_for_value[curr[switch_var]] != NULL) {
        generator_for_value[curr[switch_var]]->generate_applicable_ops(curr, ops);
    }
    if (default_generator != NULL) {
        default_generator->generate_applicable_ops(curr, ops);
    }
}

void SuccessorGeneratorSwitch::_dump(string indent)
{
    cout << indent << "switch on " << g_variable_name[switch_var] << endl;
    cout << indent << "immediately:" << endl;
    if (immediate_ops != NULL) {
        immediate_ops->_dump(indent + "  ");
    } else {
        cout << indent << "NULL" << endl;
    }
    for (int i = 0; i < var_range; ++i) {
        cout << indent << "case " << i << ":" << endl;
        if (generator_for_value[i] != NULL) {
            generator_for_value[i]->_dump(indent + "  ");
        } else {
            cout << indent << "NULL" << endl;
        }
    }
    cout << indent << "always:" << endl;
    if (default_generator != NULL) {
        default_generator->_dump(indent + "  ");
    } else {
        cout << indent << "NULL" << endl;
    }
}

void SuccessorGeneratorGenerate::generate_applicable_ops(const GlobalState &,
        vector<const GlobalOperator *> &ops)
{
    ops.insert(ops.end(), op.begin(), op.end());
}

SuccessorGeneratorGenerate::SuccessorGeneratorGenerate(istream &in)
{
    int count;
    in >> count;
    for (int i = 0; i < count; ++i) {
        int op_index;
        in >> op_index;
        op.push_back(&g_operators[op_index]);
    }
}

SuccessorGeneratorGenerate::SuccessorGeneratorGenerate()
{

}

void SuccessorGeneratorGenerate::_dump(string indent)
{
    for (size_t i = 0; i < op.size(); ++i) {
        cout << indent;
        op[i]->dump();
    }
}

SuccessorGenerator *read_successor_generator(istream &in)
{
    string type;
    in >> type;
    if (type == "switch") {
        return new SuccessorGeneratorSwitch(in);
    } else if (type == "check") {
        return new SuccessorGeneratorGenerate(in);
    }
    cout << "Illegal successor generator statement!" << endl;
    cout << "Expected 'switch' or 'check', got '" << type << "'." << endl;
    exit_with(EXIT_INPUT_ERROR);
}

SuccessorGenerator *create_successor_generator_from_vector(
    const std::vector<int> &variable_domain,
    const std::vector<GlobalOperator> &ops,
    const std::vector<std::vector<GlobalCondition> > *preconditions)
{
    assert(preconditions == NULL || preconditions->size() == ops.size());
    int root_var_index = 0;

    SuccessorGeneratorSwitch *root_node = new SuccessorGeneratorSwitch(
        root_var_index,
        variable_domain[root_var_index]);

    for (size_t op_no = 0; op_no < ops.size(); op_no++) {
        const vector<GlobalCondition> *conditions = NULL;
        if (preconditions != NULL) {
            conditions = &preconditions->at(op_no);
        } else {
            conditions = &ops[op_no].get_preconditions();
        }

        if (conditions->size() == 0) {
            // This op has no preconditions, add it immediately to the root node
            if (root_node->immediate_ops == NULL) {
                root_node->immediate_ops = new SuccessorGeneratorGenerate();
            }
            ((SuccessorGeneratorGenerate *) root_node->immediate_ops)->add_op(&ops[op_no]);
            continue;
        }

        SuccessorGeneratorSwitch *current_node = root_node;
        for (size_t cond_no = 0; cond_no < conditions->size(); cond_no++) {
            int var = conditions->at(cond_no).var;
            int val = conditions->at(cond_no).val;
            //cout << "Consider precond with var: " << var << ", val: " << val << endl;

            while (var != current_node->switch_var) {
                if (current_node->default_generator == NULL) {
                    int next_var_index = current_node->switch_var + 1;
                    current_node->default_generator = new SuccessorGeneratorSwitch(next_var_index,
                            variable_domain[next_var_index]);
                }

                current_node = (SuccessorGeneratorSwitch *) current_node->default_generator;
            }

            // Here: var == current_node->switch_var

            int next_var_index = current_node->switch_var + 1;
            if (next_var_index >= (int) variable_domain.size()) {
                if (current_node->generator_for_value[val] == NULL) {
                    current_node->generator_for_value[val] = new SuccessorGeneratorGenerate();
                }
                ((SuccessorGeneratorGenerate *) current_node->generator_for_value[val])->add_op(
                    &ops[op_no]);

            } else {
                if (current_node->generator_for_value[val] == NULL) {
                    current_node->generator_for_value[val] = new SuccessorGeneratorSwitch(
                        next_var_index,
                        variable_domain[next_var_index]);
                }

                current_node = (SuccessorGeneratorSwitch *)
                               current_node->generator_for_value[val];
                if (cond_no == (conditions->size() - 1)) {
                    // This was the last cond.
                    if (current_node->immediate_ops == NULL) {
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
