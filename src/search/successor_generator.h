#ifndef SUCCESSOR_GENERATOR_H
#define SUCCESSOR_GENERATOR_H

#include <iostream>
#include <vector>

class GlobalOperator;
class GlobalState;

class SuccessorGenerator {
public:
    virtual ~SuccessorGenerator() {}
    virtual void generate_applicable_ops(const GlobalState &curr,
                                         std::vector<const GlobalOperator *> &ops) = 0;
    void dump() {_dump("  "); }
    virtual void _dump(std::string indent) = 0;
};

class SuccessorGeneratorSwitch : public SuccessorGenerator {
public:
    int switch_var;
    int var_range;
    SuccessorGenerator *immediate_ops;
    std::vector<SuccessorGenerator *> generator_for_value;
    SuccessorGenerator *default_generator;
    SuccessorGeneratorSwitch(std::istream &in);
    SuccessorGeneratorSwitch(int _switch_var, int _var_range);
    virtual void generate_applicable_ops(const GlobalState &curr,
    		std::vector<const GlobalOperator *> &ops);
    virtual void _dump(std::string indent);
};

class SuccessorGeneratorGenerate : public SuccessorGenerator {
	std::vector<const GlobalOperator *> op;
public:
    SuccessorGeneratorGenerate(std::istream &in);
    SuccessorGeneratorGenerate();
    virtual void generate_applicable_ops(const GlobalState &curr,
    		std::vector<const GlobalOperator *> &ops);
    virtual void _dump(std::string indent);
    void add_op(const GlobalOperator *new_op) { op.push_back(new_op); }
};

SuccessorGenerator *read_successor_generator(std::istream &in);

#endif
