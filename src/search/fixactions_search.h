/*
 * fixactionssearch.h
 *
 *  Created on: 12.12.2016
 *      Author: Patrick
 */

#ifndef SRC_SEARCH_FIXACTIONS_SEARCH_H_
#define SRC_SEARCH_FIXACTIONS_SEARCH_H_

#include "search_engine.h"
#include "successor_generator.h"

class FixActionsSearch: public SearchEngine
{

protected:
    virtual void initialize();
    virtual SearchStatus step();
    int parse_success_prob_cost(std::string prob);
    void divideVariables();
    void clean_attack_actions();
    void create_new_variable_indices();
    void adjust_var_indices_of_ops(std::vector<GlobalOperator> &ops);
    SuccessorGeneratorSwitch* create_fix_vars_successor_generator(const std::vector<GlobalOperator> &pre_cond_ops, const std::vector<GlobalOperator> &ops);
    void compute_commutative_fix_ops_matrix();

public:
    FixActionsSearch(const Options &opts);
    virtual ~FixActionsSearch();
    static void add_options_to_parser(OptionParser &parser);
};

#endif /* SRC_SEARCH_FIXACTIONS_SEARCH_H_ */
