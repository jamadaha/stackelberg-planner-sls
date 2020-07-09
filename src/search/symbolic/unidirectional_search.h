#ifndef SYMBOLIC_UNIDIRECTIONAL_SEARCH_H
#define SYMBOLIC_UNIDIRECTIONAL_SEARCH_H

#include "sym_search.h"

#include "sym_estimate.h"
#include "sym_util.h"
#include "sym_solution.h"

#include <vector>
#include <map>
#include <memory>

namespace symbolic {
    class SymSolution;
    class UnidirectionalSearch;
    class SymController; 
    class SymExpStatistics {
    public:
	double image_time, image_time_failed;
	double time_heuristic_evaluation;
	int num_steps_succeeded;
	double step_time;

    SymExpStatistics() :
        image_time(0),
	    image_time_failed(0), time_heuristic_evaluation(0),
	    num_steps_succeeded(0), step_time(0) {  }


	void add_image_time(double t) {
	    image_time += t;
	    num_steps_succeeded += 1;
	}

	void add_image_time_failed(double t) {
	    image_time += t;
	    image_time_failed += t;
	    num_steps_succeeded += 1;
	}
    };


    class OppositeFrontier {
    public: 
	virtual SymSolution checkCut(const PlanReconstruction * search, const BDD &states, int g, bool fw) const = 0;
        
	virtual BDD notClosed () const = 0;

	//Returns true only if all not closed states are guaranteed to be dead ends
	virtual bool exhausted () const = 0;
	
	virtual int getHNotClosed() const = 0;
    };

    class OppositeFrontierFixed : public OppositeFrontier {
	BDD goal;
	int hNotGoal;
    public:
	OppositeFrontierFixed (BDD g, const SymStateSpaceManager & mgr); 
	virtual SymSolution checkCut(const PlanReconstruction * search, const BDD &states, int g, bool fw) const override;

	virtual BDD notClosed () const override {
	    return !goal;
	}

	virtual bool exhausted () const override {
	    return false;
	}

	virtual int getHNotClosed() const override {
	    return hNotGoal;
	}
    };

    class OppositeFrontierComposite  : public OppositeFrontier {

    public:
        std::shared_ptr<OppositeFrontier> f1, f2;
    OppositeFrontierComposite(std::shared_ptr<OppositeFrontier> f1_,
                              std::shared_ptr<OppositeFrontier> f2_) : f1(f1_), f2(f2_) {
        }

        virtual SymSolution checkCut(const PlanReconstruction * search, const BDD &states, int g, bool fw) const override;

	virtual BDD notClosed () const override {
            return f1->notClosed();
        }

	//Returns true only if all not closed states are guaranteed to be dead ends
	virtual bool exhausted () const override {
            return f1->exhausted();
        }
	
	virtual int getHNotClosed() const override {
            return f1->getHNotClosed();
        }
    };


    class UnidirectionalSearch : public SymSearch, public PlanReconstruction {
    protected:
	bool fw; //Direction of the search. true=forward, false=backward

	SymExpStatistics stats;

	std::shared_ptr<OppositeFrontier> perfectHeuristic;
    public:

	UnidirectionalSearch(SymController * eng, const SymParamsSearch &params);

    virtual ~UnidirectionalSearch() = default;

	inline bool isFW() const {
	    return fw;
	}

	void statistics() const override;

	/* virtual void getPlan(const BDD &cut, int g, std::vector <const GlobalOperator
         * *> &path) const = 0; */

        virtual SymVariables *getVars() const override;        

	virtual int getG() const = 0;

    };
}
#endif // SYMBOLIC_EXPLORATION
