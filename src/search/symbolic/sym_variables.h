#ifndef SYMBOLIC_SYM_VARIABLES_H
#define SYMBOLIC_SYM_VARIABLES_H

#include "sym_bucket.h"

#include "../utils/timer.h"
#include "../globals.h"
#include "../global_operator.h"
#include <math.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <string>
#include <map>
#include <cassert>

class Options;
class OptionParser;

namespace symbolic {
/*
 * BDD-Variables for a symbolic exploration.
 * This information is global for every class using symbolic search.
 * The only decision fixed here is the variable ordering, which is assumed to be always fixed.
 */
struct BDDError
{};
extern void exceptionError(std::string message);
extern void exitOutOfMemory(size_t memory);


class SymVariables {
    // Var order used by the algorithm.
    //const VariableOrderType variable_ordering;
    //Parameters to initialize the CUDD manager
    const long cudd_init_nodes; //Number of initial nodes
    const long cudd_init_cache_size; //Initial cache size
    const long cudd_init_available_memory; //Maximum available memory (bytes)
    const bool gamer_ordering;

    std::unique_ptr<Cudd> _manager; //_manager associated with this symbolic search

    int numBDDVars; //Number of binary variables (just one set, the total number is numBDDVars*3
    std::vector<BDD> variables; // BDD variables

    //The variable order must be complete.
    std::vector <int> var_order; //Variable(FD) order in the BDD
    std::vector <std::vector <int>> bdd_index_pre, bdd_index_eff; //vars(BDD) for each var(FD)
    
    std::vector <std::vector <BDD>> preconditionBDDs; // BDDs associated with the precondition of a predicate
    std::vector <std::vector <BDD>> effectBDDs;      // BDDs associated with the effect of a predicate
    std::vector<BDD> biimpBDDs;  //BDDs associated with the biimplication of one variable(FD)
    std::vector<BDD> validValues; // BDD that represents the valid values of all the variables
    BDD validBDD;  // BDD that represents the valid values of all the variables


    //Vector to store the binary description of an state
    //Avoid allocating memory during heuristic evaluation
    mutable std::vector <char> binStateChar; 
    mutable std::vector <int> binStateInt;
    // We need different representations because some CUDD functions use int * and some
    // use char *

public:
    SymVariables(const Options &opts);
    void init();
    void init(const std::vector <int> &v_order);


    //State getStateFrom(const BDD & bdd) const;
    BDD getStateBDD(const GlobalState &state) const;
    BDD getStateBDD(const std::vector<int> &state) const;
    BDD getPartialStateBDD(const std::vector<int> &state, const std::vector<bool> & pattern) const;
    BDD getPartialStateBDD(const std::vector<std::pair<int, int>> &state) const;
 //Returns the number of states in a BDD
    double numStates(const BDD & bdd, int relevant_vars) const;
    double numStates(const BDD &bdd) const {
        return numStates(bdd, numBDDVars); 
    }
    double numStates() const;
    double numStates(const Bucket &bucket) const;


    double percentageNumStates(const BDD &bdd) const {
        return numStates(bdd) / numStates();
    }
    bool isIn(const GlobalState &state, const BDD &bdd) const;


    inline const std::vector<int> &vars_index_pre(int variable) const {
        return bdd_index_pre[variable];
    }
    inline const std::vector<int> &vars_index_eff(int variable) const {
        return bdd_index_eff[variable];
    }

    inline const BDD &preBDD(int variable, int value) const {
        return preconditionBDDs [variable] [value];
    }

    inline const BDD &effBDD(int variable, int value) const {
        return effectBDDs [variable] [value];
    }


    inline BDD getCubePre(int var) const {
        return getCube(var, bdd_index_pre);
    }
    inline BDD getCubePre(const std::set <int> &vars) const {
        return getCube(vars, bdd_index_pre);
    }

    inline BDD getCubeEff(int var) const {
        return getCube(var, bdd_index_eff);
    }
    inline BDD getCubeEff(const std::set <int> &vars) const {
        return getCube(vars, bdd_index_eff);
    }

    inline const BDD &biimp(int variable) const {
        return biimpBDDs[variable];
    }

    inline long totalNodes() const {
        return _manager->ReadNodeCount();
    }

    inline std::vector <BDD> getBDDVarsPre() const {
        return getBDDVars(var_order, bdd_index_pre);
    }
    inline std::vector <BDD> getBDDVarsEff() const {
        return getBDDVars(var_order, bdd_index_eff);
    }
    inline std::vector <BDD> getBDDVarsPre(const std::vector <int> &vars) const {
        return getBDDVars(vars, bdd_index_pre);
    }
    inline std::vector <BDD> getBDDVarsEff(const std::vector <int> &vars) const {
        return getBDDVars(vars, bdd_index_eff);
    }

    inline unsigned long totalMemory() const {
        return _manager->ReadMemoryInUse();
    }

    inline double totalMemoryGB() const {
        return _manager->ReadMemoryInUse()/(1024*1024*1024);
    }

    inline BDD zeroBDD() const {
        return _manager->bddZero();
    }

    inline BDD oneBDD() const {
	assert(_manager);
        return _manager->bddOne();
    }

    inline BDD validStates() const {
        return validBDD;
    }

    inline Cudd *mgr() const {
        return _manager.get();
    }

    inline BDD bddVar(int index) const {
        return variables[index];
    }

    inline int usedNodes() const {
        return _manager->ReadSize();
    }

    inline void setTimeLimit(int maxTime) {
        _manager->SetTimeLimit(maxTime);
        _manager->ResetStartTime();
    }

    inline void unsetTimeLimit() {
        _manager->UnsetTimeLimit();
    }

    void print();
    
    template <class T> 
    char *getBinaryDescriptionChar(const T &state) {
        int pos = 0;
        //  cout << "State " << endl;
        for (int v : var_order) {
            //cout << v << "=" << state[v] << " " << g_variable_domain[v] << " assignments and  " << binary_len[v] << " variables   " ;
            //preconditionBDDs[v] [state[v]].PrintMinterm();

            for (size_t j = 0; j < bdd_index_pre[v].size(); j++) {
                binStateChar[pos++] = ((state[v] >> j) % 2);
                binStateChar[pos++] = 0; //Skip interleaving variable
            }
        }
        /* cout << "Binary description: ";
           for(int i = 0; i < pos; i++){
           cout << binStateChar[i];
           }
           cout << endl;*/

        return &(binStateChar[0]);
    }

    template <class T> 
    int *getBinaryDescriptionInt(const T &state) {
        int pos = 0;
        //  cout << "State " << endl;
        for (int v : var_order) {
      	//       std::cout << v << "=" << state[v] << " " << g_variable_domain[v] << " assignments  " << g_fact_names[v][state[v]] ;
            
            // preconditionBDDs[v] [state[v]].PrintMinterm();

            for (size_t j = 0; j < bdd_index_pre[v].size(); j++) {
                binStateInt[pos++] = ((state[v] >> j) % 2);
                binStateInt[pos++] = 0; //Skip interleaving variable
            }
        }
        /*std::cout << " Binary description: ";
           for(int i = 0; i < pos; i++){
               std::cout << binStateInt[i];
           }
           std::cout << std::endl;*/

        return &(binStateInt[0]);
    }


    std::vector<int> getStateDescription(const std::vector<char> & binary_state) const;
    std::vector<int> sample_state (const BDD &  bdd) const;


    std::vector<int> getStateDescription(const std::vector<char> & binary_state, const std::vector<bool> & pattern) const;
    std::vector<int> sample_state (const BDD &  bdd, const std::vector<bool> & pattern) const;


    

    inline ADD getADD(int value) {
        return _manager->constant(value);
    }

    inline ADD getADD(std::map<int, BDD> heur) {
        ADD h = getADD(-1);
        for (const auto &entry : heur) {
            int distance = 1 + entry.first;
            h += entry.second.Add() * getADD(distance);
        }
        return h;
    }

    static void add_options_to_parser(OptionParser &parser);

    void print_options() const;


    inline int getNumBDDVars() const {
        return numBDDVars;
    }

private:
    //Auxiliar function helping to create precondition and effect BDDs
    //Generates value for bddVars.
    BDD generateBDDVar(const std::vector<int> &_bddVars, int value) const;
    BDD getCube(int var, const std::vector<std::vector<int>> &v_index) const;
    BDD getCube(const std::set <int> &vars, const std::vector<std::vector<int>> &v_index) const;
    BDD createBiimplicationBDD(const std::vector<int> &vars, const std::vector<int> &vars2) const;
    std::vector <BDD> getBDDVars(const std::vector <int> &vars, const std::vector<std::vector<int>> &v_index) const;


    inline BDD createPreconditionBDD(int variable, int value) const {
        return generateBDDVar(bdd_index_pre[variable], value);
    }

    inline BDD createEffectBDD(int variable, int value) const {
        return generateBDDVar(bdd_index_eff[variable], value);
    }

};
}

#endif
