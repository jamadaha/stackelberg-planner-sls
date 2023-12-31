#include "sym_controller.h"

#include "opt_order.h"

#include "sym_state_space_manager.h"
#include "bidirectional_search.h"
#include "hnode.h"
#include "../utils/debug_macros.h"
#include "../globals.h"
// #include "../merge_and_shrink/ld_simulation.h"
// #include "../merge_and_shrink/abstraction_builder.h"


#include "../option_parser.h"

using namespace std;

namespace symbolic {

    SymController::SymController(std::shared_ptr<SymVariables> v,
                                 const SymParamsMgr &pMgr, const SymParamsSearch & pSearch) : vars(v), mgrParams(pMgr), searchParams(pSearch), lower_bound(0), upper_bound(std::numeric_limits<int>::max()),
                                                                                              print_bound(false) {}

    SymController::SymController(const Options &opts)
        : vars(make_shared<SymVariables>(opts)),
          mgrParams(opts), searchParams(opts), lower_bound(0), upper_bound(std::numeric_limits<int>::max()), print_bound(true){
        mgrParams.print_options();
        searchParams.print_options();

        //TODO: This should be done before computing the var order and
        //initializing vars. Done here to avoid memory errors
        // if(abstractionBuilder) {
        //  unique_ptr<LDSimulation> ldSim;
        //  std::vector<std::unique_ptr<Abstraction> > abstractions;
        //  // TODO: This irrelevance pruning is only safe for detecting unsolvability
        //  abstractionBuilder->build_abstraction
        //      (true, OperatorCost::ZERO, ldSim, abstractions);
        //  cout << "LDSimulation finished" << endl;

        //  ldSim->release_memory();
        // }

        vars->init();
    }


    void SymController::add_options_to_parser(OptionParser &parser, int maxStepTime, int maxStepNodes) {
	SymVariables::add_options_to_parser(parser);
	SymParamsMgr::add_options_to_parser(parser);
	SymParamsSearch::add_options_to_parser(parser, maxStepTime, maxStepNodes);
    }
    void SymController::new_solution(const SymSolution & sol) {
	if(!solution.solved() ||
	   sol.getCost() < solution.getCost()){
	    solution = sol;

            if (print_bound) {
                std::cout << "BOUND: " << lower_bound << " < " << getUpperBound()
                          << ", total time: " << g_timer << std::endl;
            }

	}
    }

    void SymController::setLowerBound(int lower) {
	//Never set a lower bound greater than the current upper bound
	if(solution.solved()) {
	    lower = min(lower,  solution.getCost());
	}

	if(lower > lower_bound) {
	    lower_bound = lower;

            if (print_bound) {
                std::cout << "BOUND: " << lower_bound << " < " << getUpperBound()
                          << ", total time: " << g_timer << std::endl;
            }

	}

    }
}
