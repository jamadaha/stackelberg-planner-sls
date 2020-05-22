#include "sym_solution.h"

#include <vector>       // std::vector
#include "../state_registry.h"
#include "../utils/debug_macros.h"

#include "unidirectional_search.h"


using namespace std;

namespace symbolic {

    void SymSolution::getPlan(vector <const GlobalOperator *> &path) const {
        getPlan(path, g_initial_state_data, vector<bool>());
    }

    void SymSolution::getPlan(vector <const GlobalOperator *> &path, const vector<int> & initial_state, const vector<bool> & pattern) const {
	assert (path.empty()); //This code should be modified to allow appending things to paths
	DEBUG_MSG(cout << "Extract path forward: " << g << endl; );
	if (exp_fw) {
	    exp_fw->getPlan(cut, g, true, path);   
	}
	DEBUG_MSG(cout << "Extract path backward: " << h << endl << " starting from path: "  << endl;
                  for (const auto *  op : path ) {
                      cout << op->get_name() << endl;
                  }
                  cout << "Starting state" << endl;
                  for (size_t v = 0; v < g_fact_names.size(); ++v) {
                      if (pattern[v]) {
                          cout << g_fact_names[v][initial_state[v]] << endl;
                      }
                  }
            );

        
	if (exp_bw) {
	    BDD newCut;
	    if (!path.empty()) {
		auto s = initial_state;
		//Get state
		for (auto op : path) {
                    for (const GlobalEffect &eff : op->get_effects()) {
			if (eff.does_fire(s)) {
			    s[eff.var] = eff.val;
			}
		    }
		}
                if(pattern.empty()) {
                    newCut = exp_bw->getVars()->getStateBDD(s);
                } else {
                    newCut = exp_bw->getVars()->getPartialStateBDD(s, pattern);
                }
	    } else {
		newCut = cut;
	    }
    
	    exp_bw->getPlan(newCut, h, false, path);
	}
	/*DEBUG_MSG(cout << "Path extracted" << endl;
	  State s2 (*g_initial_state);
	  //Get state
	  for(auto op : path){
	  cout << op->get_name() << endl;
	  if(!op->is_applicable(s2)){
	  cout << "ERROR: bad plan reconstruction" << endl;
	  cout << op->get_name() << " is not applicable" << endl;
	  exit(-1);
	  }
	  s2 = State(s2, *op);
	  }
	  if(!test_goal(s2)){
	  cout << "ERROR: bad plan reconstruction" << endl;
	  cout << "The plan ends on a non-goal state" << endl;
	  exit(-1);
	  });*/
        
    
    }

    ADD SymSolution::getADD() const {
	assert(exp_fw || exp_bw);
	vector <const GlobalOperator *> path;
	getPlan(path);

	SymVariables *vars = nullptr;
	if(exp_fw) vars = exp_fw->getVars();
	else if(exp_bw) vars = exp_bw->getVars();
	
	ADD hADD = vars->getADD(-1);
	int h_val = g + h;

        auto s = g_initial_state_data;
	BDD sBDD = vars->getStateBDD(s);
	hADD += sBDD.Add() * (vars->getADD(h_val + 1));
	for (auto op : path) {
	    h_val -= op->get_cost();
	    for (const GlobalEffect &eff : op->get_effects()) {
		if (eff.does_fire(s)) {
		    s[eff.var] = eff.val;
		}
	    }
	    sBDD = vars->getStateBDD(s);
	    hADD += sBDD.Add() * (vars->getADD(h_val + 1));
	}
	return hADD;
    }
}
