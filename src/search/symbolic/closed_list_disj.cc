#include "closed_list_disj.h"

#include "sym_state_space_manager.h"
#include "sym_solution.h"
#include "sym_util.h"
#include "sym_astar.h"
#include "sym_bucket.h"

#include "../utils/timer.h"
#include "../utils/debug_macros.h"
#include "closed_list.h"

#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include "../successor_generator.h"

#include "sym_test.h"

#include <cassert>

using namespace std;

namespace symbolic {

    ClosedListDisj::ClosedListDisj() : mgr(nullptr) {
    }

    ClosedListDisj::ClosedListDisj (const ClosedListDisj & other, const BDD & subset) : mgr(other.mgr) {
        for (const auto & entry  : other.closed){
            int hval = entry.first;
            for (const BDD & bdd : entry.second) {
                BDD result = bdd*subset;            
                if (!result.IsZero()) {
                    closed[hval].push_back(result);
                    h_values.insert(hval);
                    if (other.zeroCostClosed.count(hval)) {
                        for (auto & entry : other.zeroCostClosed.at(hval)) {
                            zeroCostClosed[hval].push_back(Bucket());
                            for (const auto & bdd : entry){
                                zeroCostClosed[hval].back().push_back(bdd*subset);
                            }
                        }
                    }
                }
            }
            // assert(other.closedUpTo.count(hval));
            // closedUpTo[hval] = other.closedUpTo.at(hval)*subset;        
        }

        for (const BDD & bdd : other.closedTotal) {
            closedTotal.push_back(bdd * subset);
        }
        hNotClosed = 0;
        fNotClosed = 0;        
    }

    void ClosedListDisj::load(const ClosedListDisj &other) {
        for (const auto & entry  : other.closed) {
            int hval = entry.first;
            for (const BDD & bdd : entry.second) {
                closed[hval].push_back(bdd);
            }

            h_values.insert(hval);
            if (other.zeroCostClosed.count(hval)) {
                const auto & bdds = other.zeroCostClosed.at(hval);
                for(size_t i = 0; i < bdds.size(); ++i) {
                    if  (i >= zeroCostClosed[hval].size()) {
                        zeroCostClosed[hval].push_back(Bucket());
                    }                    
                    for (const BDD & bdd : bdds[i]) {
                        zeroCostClosed[hval][i].push_back(bdd);
                    }

                }
            }
        }
        for (const BDD & bdd : other.closedTotal) {
            closedTotal.push_back(bdd);
        }
    }

     void ClosedListDisj::load(const ClosedList &other) {
        for (const auto & entry  : other.closed) {
            int hval = entry.first;
            closed[hval].push_back(entry.second);

            h_values.insert(hval);
            if (other.zeroCostClosed.count(hval)) {
                const auto & bdds = other.zeroCostClosed.at(hval);
                for(size_t i = 0; i < bdds.size(); ++i) {
                    if  (i >= zeroCostClosed[hval].size()) {
                        zeroCostClosed[hval].push_back(Bucket());
                    }                    

                    zeroCostClosed[hval][i].push_back(bdds[i]);
                }
            }
        }

        closedTotal.push_back(other.closedTotal);
        
    }

    void ClosedListDisj::init(SymStateSpaceManager *manager) {
	mgr = manager;
	set<int>().swap(h_values);
	// map<int, BDD>().swap(closedUpTo);
	map <int, vector<Bucket>>().swap(zeroCostClosed);
	map<int, Bucket>().swap(closed);
	hNotClosed = 0;
	fNotClosed = 0;
    }


    // void ClosedListDisj::init(SymStateSpaceManager *manager, const ClosedListDisj &other) {
    //     mgr = manager;
    //     set<int>().swap(h_values);
    //     // map<int, BDD>().swap(closedUpTo);
    //     map <int, vector<Bucket>>().swap(zeroCostClosed);
    //     map<int, Bucket>().swap(closed);
    //     hNotClosed = 0;
    //     fNotClosed = 0;

    //     closedTotal = mgr->shrinkForall(other.closedTotal);
    //     closed[0] = closedTotal;
    //     newHValue(0);	
    // }

    void ClosedListDisj::newHValue(int h_value) {
	h_values.insert(h_value);
    }

    void ClosedListDisj::insert(int h, const BDD &S) {
	DEBUG_MSG(cout << "Inserting on closed "  << "g=" << h << ": " << S.nodeCount() << " nodes and "
		  << mgr->getVars()->numStates(S) << " states" << endl;
	    );

#ifdef DEBUG_GST
	gst_plan.checkClose(S, h, exploration);
#endif

        if (!closed.count(h)) {
	    newHValue(h);
	}

        closed[h].push_back(S);

	if (mgr->hasTransitions0()) {
	    zeroCostClosed[h].push_back(Bucket(S));
	}
	closedTotal.push_back(S);

        
	// //Introduce in closedUpTo
	// auto c = closedUpTo.lower_bound(h);
	// while (c != std::end(closedUpTo)) {
	//     c->second += S;
	//     c++;
	// }
        // Warning: We should introduce items in closedUpTo
    }


    void ClosedListDisj::insertWithZeroCostSteps(int h, int zeroCostSteps, const BDD &S) {
	DEBUG_MSG(cout << "Inserting on closed "  << "g=" << h << ": " << S.nodeCount() << " nodes and "
		  << mgr->getVars()->numStates(S) << " states" << endl;
	    );

#ifdef DEBUG_GST
	gst_plan.checkClose(S, h, exploration);
#endif

        
        if (!closed.count(h)) {
	    newHValue(h);
	}

        closed[h].push_back(S);

	if (mgr->hasTransitions0()) {
            assert ((size_t)zeroCostSteps <= zeroCostClosed[h].size());
            if ((size_t)zeroCostSteps >= zeroCostClosed[h].size()){
                zeroCostClosed[h].push_back(Bucket());
            }
            zeroCostClosed[h][zeroCostSteps].push_back(S);
	}
        
	closedTotal.push_back(S);

	// //Introduce in closedUpTo
	// auto c = closedUpTo.lower_bound(h);
	// while (c != std::end(closedUpTo)) {
	//     c->second += S;
	//     c++;
	// }
        // Warning: We should introduce items in closedUpTo
    }


    void ClosedListDisj::setHNotClosed(int newHNotClosed) {
	if (newHNotClosed > hNotClosed) {
	    hNotClosed = newHNotClosed;
	    newHValue(newHNotClosed); //Add newHNotClosed to list of h values (and those of parents)
	}
    }

    void ClosedListDisj::setFNotClosed(int f) {
	if (f > fNotClosed) {
	    fNotClosed = f;
	}
    }

    void ClosedListDisj::extract_path(const BDD &c, int h, bool fw,
                                  vector <const GlobalOperator *> &path) const {
	if (!mgr) {
	    return;
        }

	const map<int, vector<TransitionRelation>> &trs = mgr->getIndividualTRs();

	BDD cut = c;
	size_t steps0 = 0;
	if (zeroCostClosed.count(h)) {
	    assert(trs.count(0));
	    DEBUG_MSG(cout << "Check " << steps0 << " of " << zeroCostClosed.at(h).size() << endl;);
	    while (steps0 < zeroCostClosed.at(h).size() && empty_intersection(zeroCostClosed.at(h)[steps0], cut)) {
		//DEBUG_MSG(cout << "Steps0 is not " << steps0<< " of " << zeroCostClosed.at(h).size() << endl;);
		steps0++;
	    }
	    //DEBUG_MSG(cout << "Steps0 of h=" << h << " is " << steps0 << endl;);
	    if (steps0 < zeroCostClosed.at(h).size()) {
		cut  = get_non_empty_intersection(zeroCostClosed.at(h)[steps0], cut);
	    } else {
		DEBUG_MSG(cout << "cut not found with steps0. Try to find with preimage: " << trs.count(0) << endl;
		    );
		bool foundZeroCost = false;
		for (const TransitionRelation &tr : trs.at(0)) {
		    if (foundZeroCost)
			break;
		    BDD succ;
		    if (fw) {
			succ = tr.preimage(cut);
		    } else {
			succ = tr.image(cut);
		    }
		    if (succ.IsZero()) {
			continue;
		    }

		    for (size_t newSteps0 = 0; newSteps0 < steps0; newSteps0++) {
			BDD intersection = get_non_empty_intersection(zeroCostClosed.at(h)[newSteps0], succ);
			if (!intersection.IsZero()) {
			    steps0 = newSteps0;
			    cut = succ;
			    //DEBUG_MSG(cout << "Adding " << (*(tr.getOps().begin()))->get_name() << endl;);
			    path.push_back(*(tr.getOps().begin()));
			    foundZeroCost = true;
			    break;
			}
		    }
		}
		if (!foundZeroCost) {
		    DEBUG_MSG(cout << "cut not found with steps0. steps0=0:" << endl;
			);

		    steps0 = 0;
		}
	    }
	}

	while (h > 0 || steps0 > 0) {
	    DEBUG_MSG(
		cout << "h=" << h << " and steps0=" << steps0 << endl;
		//cout << "CUT: "; cut.print(0, 1);
		);
	    if (steps0 > 0) {
		bool foundZeroCost = false;
		//Apply 0-cost operators
		if (trs.count(0)) {
		    for (const TransitionRelation &tr : trs.at(0)) {
			if (foundZeroCost)
			    break;
			BDD succ;
			if (fw) {
			    succ = tr.preimage(cut);
			} else {
			    succ = tr.image(cut);
			}
			if (succ.IsZero()) {
			    continue;
			}

			DEBUG_MSG(cout << "SUCC: ";
				  succ.print(0, 1);
			    );
			for (size_t newSteps0 = 0; newSteps0 < steps0; newSteps0++) {
			    BDD intersection = get_non_empty_intersection(zeroCostClosed.at(h)[newSteps0], succ);
			    if (!intersection.IsZero()) {
				steps0 = newSteps0;
				cut = succ;
				//DEBUG_MSG(cout << "Adding " << (*(tr.getOps().begin()))->get_name() << endl;);
				path.push_back(*(tr.getOps().begin()));
				foundZeroCost = true;
				break;
			    }
			}
		    }
		}

		if (!foundZeroCost) {
		    /*    DEBUG_MSG(
			  cout << "Force steps0 = 0" << endl;
			  for (int newSteps0 = 0; newSteps0 <= steps0; newSteps0++){
			  cout << "Steps0: " << newSteps0 << ": "; zeroCostClosed.at(h)[newSteps0].print(0,2);
			  cout << "CUT: "; cut.print(0, 1);
			  }
			  );*/
		    steps0 = 0;
		}
	    }

	    if (h > 0 && steps0 == 0) {
		bool found = false;

                assert (!trs.empty());
		for (auto key : trs) { //TODO: maybe is best to use the inverse order
		    if (found) {
			break;
                    }
                    DEBUG_MSG(cout << "Trying: " << key.first << endl;);
		    int newH = h - key.first;
		    if (key.first == 0 || closed.count(newH) == 0)
			continue;
                    DEBUG_MSG(cout << "Trying2: " << key.first << " " << key.second << endl;);
                    assert (!key.second.empty());
		    for (TransitionRelation &tr : key.second) {
			//DEBUG_MSG(cout << "Check " << tr.getOps().size() << " " << (*(tr.getOps().begin()))->get_name() << " of cost " << key.first << " in h=" << newH << endl;);
                        assert(tr.getOps().size() == 1);
                        DEBUG_MSG(cout << "Check " << (*(tr.getOps().begin()))->get_name() << endl;);
                        
			BDD succ;
			if (fw) {
			    succ = tr.preimage(cut);
			} else {
			    succ = tr.image(cut);
			}

                        
			
			/*DEBUG_MSG(cout << "Image computed: "; succ.print(0,1);
			  cout << "closed at newh: "; closed.at(newH).print(0,1);
			  cout << "Intersection: "; intersection.print(0,1););*/
			if (!empty_intersection(closed.at(newH), succ)) {
                            BDD intersection = get_non_empty_intersection(closed.at(newH), succ);
			    h = newH;
			    cut = succ;
			    steps0 = 0;
			    if (zeroCostClosed.count(h)) {
				while (empty_intersection(zeroCostClosed.at(h)[steps0], cut)){
				    //DEBUG_MSG(cout << "r Steps0 is not " << steps0<< " of " << zeroCostClosed.at(h).size() << endl;);
				    steps0++;
				    assert(steps0 < zeroCostClosed.at(newH).size());
				}

				//DEBUG_MSG(cout << "r Steps0 of h=" << h << " is " << steps0 << endl;);

				cut = get_non_empty_intersection(zeroCostClosed.at(h)[steps0], cut);
			    }
			    path.push_back(*(tr.getOps().begin()));

			    //DEBUG_MSG(cout << "Selected " << path.back()->get_name() << endl;);

			    found = true;
			    break;
			}
		    }
		}
		if (!found) {
		    cerr << "Error: Solution reconstruction failed: " << endl;
                    
		    exit_with(EXIT_CRITICAL_ERROR);
		}
	    }
	}

	DEBUG_MSG(cout << "Sym closed extracted path" << endl;
	    );
    }

    SymSolution ClosedListDisj::checkCut(const PlanReconstruction * search, const BDD &states, int g, bool fw) const {
        if (empty_intersection(closedTotal, states)) {
            return SymSolution(); //No solution yet :(
        }
	BDD cut_candidate = get_non_empty_intersection(closedTotal, states);

	for (const auto &closedH : closed) {
	    int h = closedH.first;

	    DEBUG_MSG(cout << "Check cut of g=" << g << " with h=" << h << endl;);
            if (!empty_intersection(closedH.second, cut_candidate)) {
                BDD cut = get_non_empty_intersection(closedH.second, cut_candidate);
            
                if (!cut.IsZero()) {
                    if (fw) { //Solution reconstruction will fail
                        return SymSolution(search, this, g, h, cut);
                    } else{
                        return SymSolution(this, search, h, g, cut);
                    }
                }
            }
          
	}

	cerr << "Error: Cut with closedTotal but not found on closed" << endl;
	exit_with(EXIT_CRITICAL_ERROR);
    }

    // void ClosedListDisj::getHeuristic(vector<ADD> &heuristics,
    //                               vector <int> &maxHeuristicValues) const {
    //     int previousMaxH = 0; //Get the previous value of max h
    //     if (!maxHeuristicValues.empty()) {
    //         previousMaxH = maxHeuristicValues.back();
    //     }
    //     /*If we did not complete one step, and we do not surpass the previous maxH
    //       we do not have heuristic*/
    //     if (closed.size() <= 1 && hNotClosed <= previousMaxH) {
    //         cout << "Heuristic not inserted: "
    //     	 << hNotClosed << " " << closed.size() << endl;
    //         return;
    //     }

    //     ADD h = getHeuristic(previousMaxH);

    //     //  closed.clear(); //The heuristic values have been stored in the ADD.
    //     cout << "Heuristic with maxValue: " << hNotClosed
    //          << " ADD size: " << h.nodeCount() << endl;

    //     maxHeuristicValues.push_back(hNotClosed);
    //     heuristics.push_back(h);
    // }


    // ADD ClosedListDisj::getHeuristic(int previousMaxH /*= -1*/) const {
    //     /* When zero cost operators have been expanded, all the states in non reached
    //        have a h-value strictly greater than frontierCost.
    //        They can be frontierCost + min_action_cost or the least bucket in open. */
    //     /*  int valueNonReached = frontierCost;
    //         if(frontierCost >= 0 && zeroCostExpanded){
    //         cout << "Frontier cost is " << frontierCost << endl;
    //         closed[frontierCost] = S;
    //         valueNonReached = frontierCost + mgr->getMinTransitionCost();
    //         if(!open.empty()){
    //         valueNonReached = min(open.begin()->first,
    //         valueNonReached);
    //         }
    //         }*/
    //     BDD statesWithHNotClosed = !closedTotal;
    //     ADD h = mgr->mgr()->constant(-1);
    //     //cout << "New heuristic with h [";
    //     for (auto &it : closed) {
    //         //cout << it.first << " ";
    //         int h_val = it.first;

    //         /*If h_val < previousMaxH we can put it to that value
    //           However, we only do so if it is less than hNotClosed
    //           (or we will think that we have not fully determined the value)*/
    //         if (h_val < previousMaxH && previousMaxH < hNotClosed) {
    //     	h_val = previousMaxH;
    //         }
    //         if (h_val != hNotClosed) {

    //     	h += it.second.Add() * mgr->mgr()->constant(h_val + 1);
    //         } else {
    //     	statesWithHNotClosed += it.second;
    //         }
    //     }
    //     //cout << hNotClosed << "]" << endl;

    //     if (hNotClosed != numeric_limits<int>::max() && hNotClosed >= 0 && !statesWithHNotClosed.IsZero()) {
    //         h += statesWithHNotClosed.Add() * mgr->mgr()->constant(hNotClosed + 1);
    //     }

    //     return h;
    // }


    void ClosedListDisj::statistics() const {
	// cout << "h (eval " << num_calls_eval << ", not_closed" << time_eval_states << "s, closed " << time_closed_states
	//   << "s, pruned " << time_pruned_states << "s, some " << time_prune_some
	//   << "s, all " << time_prune_all  << ", children " << time_prune_some_children << "s)";
    }

    const std::set<int> &ClosedListDisj::getHValues() {
	assert(h_values.count(hNotClosed));
    
	return h_values;
    }


    // double ClosedListDisj::average_hvalue() const {
    //     double averageHeuristic = 0;
    //     double heuristicSize = 0; 
    //     for (const auto & item : closed) {
    //         double currentSize = mgr->getVars()->numStates(item.second);
    //         DEBUG_MSG(cout << item.first << " " << currentSize << endl;);
    //         averageHeuristic += currentSize * item.first;
    //         heuristicSize += currentSize;
    //     }
    //     double notClosedSize = mgr->getVars()->numStates(notClosed());
    //     heuristicSize += notClosedSize;
    //     int maxH = (closed.empty() ? 0 : closed.rbegin()->first);
    
    //     DEBUG_MSG(cout << maxH << " " << notClosedSize << endl;
    //     	  cout << "Max size: " << heuristicSize << endl << endl;);
        
    //     averageHeuristic += notClosedSize * maxH;
    //     return averageHeuristic / heuristicSize;
    // }

    void ClosedListDisj::getPlan(const BDD &cut, int g, bool fw, std::vector <const GlobalOperator *> &path) const {
	extract_path(cut, g, fw, path);
    	if (fw) {
    	    std::reverse(path.begin(), path.end());
    	} 
    }
    SymVariables *ClosedListDisj::getVars() const {
        return mgr->getVars();
    }


    int ClosedListDisj::check_goal_cost(const GlobalState & state) const {
        int * inputs = mgr->getBinaryDescription(state);

        bool found = false;

        for (const auto & bdd : closedTotal) {
            if (!bdd.Eval(inputs).IsZero()) {
                found = true;
                break;
            }
        }

        if (!found){
            return std::numeric_limits<int>::max();
        }
        
        for (const auto &closedH : closed) {
            int h = closedH.first;

            for (const auto &bdd : closedH.second) {
                if(!bdd.Eval(inputs).IsZero()) {
                    return h;
                }
            }
        }

        cerr << "Error: Cut with closedTotal but not found on closed" << endl;
        exit_with(EXIT_CRITICAL_ERROR);
                

    }

}
