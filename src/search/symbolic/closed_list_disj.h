#ifndef SYMBOLIC_CLOSED_LIST_DISJ_H
#define SYMBOLIC_CLOSED_LIST_DISJ_H

#include "sym_variables.h"
#include "unidirectional_search.h"
#include "sym_solution.h"
#include "sym_bucket.h"

#include <vector>
#include <set>
#include <map>

namespace symbolic {

class SymStateSpaceManager;
class SymSolution;
class UnidirectionalSearch;
class SymSearch;
class ClosedList;

class ClosedListDisj : public OppositeFrontier, PlanReconstruction { 
private:
    SymStateSpaceManager *mgr;  //Symbolic manager to perform bdd operations

    std::map<int, Bucket> closed;   // Mapping from cost to set of states

    // Auxiliar BDDs for the number of 0-cost action steps
    // ALERT: The information here might be wrong
    // It is just used to extract path more quickly, but the information
    // here is an (admissible) estimation and this should be taken into account
    std::map<int, std::vector<Bucket>> zeroCostClosed;
    Bucket closedTotal;             // All closed states.

    int hNotClosed, fNotClosed; // Bounds on h and g for those states not in closed
    //std::map<int, BDD> closedUpTo;  // Disjunction of BDDs in closed  (auxiliar useful to take the maximum between several BDDs)
    std::set<int> h_values; //Set of h_values of the heuristic

    void newHValue(int h_value); 

public:
    ClosedListDisj();

    ClosedListDisj(const ClosedListDisj & other, const BDD & subset);

    virtual ~ClosedListDisj() = default;

    
    void init(SymStateSpaceManager *manager);
    void init(SymStateSpaceManager *manager, const ClosedListDisj &other);

    /* void load(const ClosedListDisj &other); */

    void load(const ClosedList &other);

    /* void insert(int h, const BDD &S); */
    void insertWithZeroCostSteps(int h, int zero_cost_steps, const BDD &S);
    void setHNotClosed(int h);
    void setFNotClosed(int f);
    
    const std::set<int> &getHValues();

    //Check if any of the states is closed.
    //In case positive, return a solution pair <f_value, S>
    virtual SymSolution checkCut(const PlanReconstruction * search, const BDD &states, int g, bool fw) const override;

    void extract_path(const BDD &cut, int h, bool fw, std::vector <const GlobalOperator *> &path) const;

    /* inline Bucket getClosed() const { */
    /*     return closedTotal; */
    /* } */

    virtual BDD notClosed() const override {
        exit(0);
        return !closedTotal[0];
    }

    /* inline const std::map<int, Bucket> & getClosedList() const { */
    /*     return closed; */
    /* } */

    inline int getHNotClosed() const override {
        return hNotClosed;
    }

    inline int getFNotClosed() const {
        return fNotClosed;
    }

    /* ADD getHeuristic(int previousMaxH = -1) const; */

    /* void getHeuristic(std::vector<ADD> &heuristics, */
    /*                   std::vector <int> &maxHeuristicValues) const; */

    void statistics() const;


    /* double average_hvalue() const; */

    virtual bool exhausted () const override {
	return fNotClosed == std::numeric_limits<int>::max();
    }

    // From plan reconstruction: 
    virtual void getPlan(const BDD &cut, int g, bool fw, std::vector <const GlobalOperator *> &path) const override;
    virtual SymVariables *getVars() const override;
    virtual const std::map<int, std::vector<symbolic::TransitionRelation>> & get_transition_relation() const override;


    int check_goal_cost(const GlobalState & state) const;


};
}

#endif // SYM_CLOSED
