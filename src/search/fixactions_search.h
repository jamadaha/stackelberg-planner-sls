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
#include "attack_success_prob_reuse_heuristic.h"

template<typename T1, typename T2, typename T3> using triple = std::tuple<T1, T2, T3>;

struct FixSearchInfo {
        int attack_plan_prob_cost;
        FixSearchInfo(int _attack_plan_prob_cost = -1)
        : attack_plan_prob_cost(_attack_plan_prob_cost){ }
};



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
    SuccessorGeneratorSwitch* create_successor_generator(const std::vector<int> &variable_domain, const std::vector<GlobalOperator> &pre_cond_ops, const std::vector<GlobalOperator> &ops);
    void compute_commutative_fix_ops_matrix();
    void expand_all_successors(const GlobalState &state, std::vector<const GlobalOperator*> &op_sequence, std::vector<int> &sleep,
    		bool use_partial_order_reduction);
    void add_node_to_pareto_frontier(triple<int, int, std::vector<std::vector<const GlobalOperator*>>> &node);

public:
    FixActionsSearch(const Options &opts);
    virtual ~FixActionsSearch();
    static void add_options_to_parser(OptionParser &parser);
};

#endif /* SRC_SEARCH_FIXACTIONS_SEARCH_H_ */
