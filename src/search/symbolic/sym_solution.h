#ifndef SYMBOLIC_SYM_SOLUTION_H
#define SYMBOLIC_SYM_SOLUTION_H

#include "sym_variables.h"
#include <vector>

namespace symbolic {
    class PlanReconstruction {

    public: 
	virtual void getPlan(const BDD &cut, int g, bool fw,
                             std::vector <const GlobalOperator *> &path) const = 0;

        virtual SymVariables *getVars() const = 0;
            
        
    };

class SymSolution {
    const PlanReconstruction *exp_fw, *exp_bw;
    int g, h;
    BDD cut;
public:
    SymSolution() : g(-1), h(-1) {} //No solution yet

    SymSolution(const PlanReconstruction *e_fw, const PlanReconstruction *e_bw, int g_val, int h_val, BDD S) : exp_fw(e_fw), exp_bw(e_bw), g(g_val), h(h_val), cut(S) {}

    void getPlan(std::vector <const GlobalOperator *> &path, const std::vector<int> & initial_state, const std::vector<bool> & pattern) const;

    void getPlan(std::vector <const GlobalOperator *> &path) const;

    ADD getADD() const;

    inline bool solved() const {
        return g + h >= 0;
    }

    inline int getCost() const {
        return g + h;
    }

    inline int getCutCostFw() const {
        return g;
    }

    inline int getCutCostBw() const {
        return h;
    }


    const BDD & getCut() const {
        return cut;
    }

};
}
#endif
