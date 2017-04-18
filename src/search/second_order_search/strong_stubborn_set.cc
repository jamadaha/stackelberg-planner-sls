
#include "strong_stubborn_set.h"
#include "globals.h"
#include "../globals.h"

#include "../global_state.h"
#include "../global_operator.h"

#include "../option_parser.h"
#include "../plugin.h"

#include <algorithm>
#include <iostream>

namespace second_order_search
{

void StrongStubbornSet::initialize()
{
    std::cout << "Initializing 2OT SSS..." << std::endl;

    m_is_relevant.resize(g_outer_operators.size());
    m_operator_negated_by.resize(g_inner_operators.size());

    std::vector<std::vector<std::vector<unsigned> > > fact_neg_by(
        g_outer_variable_domain.size());
    for (unsigned var = 0; var < g_outer_variable_domain.size(); var++) {
        fact_neg_by[var].resize(g_outer_variable_domain[var]);
    }

    std::vector<int> prec(g_outer_variable_domain.size());
    for (unsigned opi = 0; opi < g_outer_operators.size(); opi++) {
        std::fill(prec.begin(), prec.end(), -1);
        const GlobalOperator &op = g_outer_operators[opi];
        for (const auto &p : op.get_preconditions()) {
            prec[p.var] = p.val;
        }
        for (const auto &e : op.get_effects()) {
            if (prec[e.var] == -1) {
                for (int val = 0; val < g_outer_variable_domain[e.var]; val++) {
                    if (val != e.val) {
                        fact_neg_by[e.var][val].push_back(opi);
                    }
                }
            } else {
                fact_neg_by[e.var][prec[e.var]].push_back(opi);
            }
        }
    }

    for (unsigned opi = 0; opi < g_inner_operator_outer_conditions.size(); opi++) {
        std::fill(m_is_relevant.begin(), m_is_relevant.end(), false);
        for (const auto &c : g_inner_operator_outer_conditions[opi]) {
            for (unsigned x : fact_neg_by[c.var][c.val]) {
                if (!m_is_relevant[x]) {
                    m_is_relevant[x] = true;
                    m_operator_negated_by[opi].push_back(x);
                }
            }
        }
    }
}

void StrongStubbornSet::prune_successors(const GlobalState &,
        const std::vector<const GlobalOperator *> &inner_plan,
        std::vector<const GlobalOperator *> &aops)
{
    std::fill(m_is_relevant.begin(), m_is_relevant.end(), false);
    for (const GlobalOperator *op : inner_plan) {
        for (unsigned x : m_operator_negated_by[op->get_op_id()]) {
            m_is_relevant[x] = true;
        }
    }
    unsigned j = 0;
    for (unsigned i = 0; i < aops.size(); i++) {
        if (m_is_relevant[aops[i]->get_op_id()]) {
            if (i != j) {
                aops[j] = aops[i];
            }
            j++;
        }
    }
    aops.resize(j);
}

}

static second_order_search::SuccessorPruningMethod *_parse(OptionParser &parser)
{
    if (!parser.dry_run()) {
        return new second_order_search::StrongStubbornSet;
    }
    return NULL;
}

static Plugin<second_order_search::SuccessorPruningMethod> _plugin("2ots3",
        _parse);
