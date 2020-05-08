#ifndef SYMBOLIC_SYM_CONTROLLER_H
#define SYMBOLIC_SYM_CONTROLLER_H

//Shared class for SymEngine and smas_heuristic

#include "sym_state_space_manager.h"
#include "sym_enums.h"
#include "sym_params_search.h"
#include "sym_solution.h"

#include <vector>
#include <memory>
#include <limits>

class OptionParser;
class Options;


namespace symbolic {
class SymSolution;
class SymVariables;
class SymPH;


class SymController {
protected:
    std::shared_ptr<SymVariables> vars; //The symbolic variables are declared here

    SymParamsMgr mgrParams; //Parameters for SymStateSpaceManager configuration.
    SymParamsSearch searchParams; //Parameters to search the original state space

    int lower_bound;
    int upper_bound;
    SymSolution solution;

    bool print_bound; 
public:
    SymController(const Options &opts);
    SymController(std::shared_ptr<SymVariables> vars,
                  const SymParamsMgr &pMgr, const SymParamsSearch & pSearch);
    
    virtual ~SymController() {}

    virtual void new_solution(const SymSolution & sol);
    void setLowerBound(int lower);

    void reset_bounds () {
        lower_bound = 0;
        upper_bound = std::numeric_limits<int>::max();
        solution = SymSolution();
    }

    int getUpperBound() const {
	if(solution.solved()) return std::min(solution.getCost(), upper_bound);
	else return upper_bound;
    }
    int getLowerBound() const {
	return lower_bound; 
    }
    
    bool solved() const {
	return getLowerBound() >= getUpperBound(); 
    }

    void set_upper_bound(int up) {
        upper_bound = std::min(upper_bound, up);
    }

    const SymSolution * get_solution () const {
	return &solution;
    }

    inline SymVariables *getVars() {
        return vars.get();
    }

    inline const SymParamsMgr &getMgrParams() const {
        return mgrParams;
    }

    inline const SymParamsSearch &getSearchParams() const {
        return searchParams;
    }

    static void add_options_to_parser(OptionParser &parser,
                                      int maxStepTime, int maxStepNodes);
};
}
#endif
