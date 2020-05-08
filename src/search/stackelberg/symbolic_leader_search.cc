#include "symbolic_leader_search.h"

#include "util.h"

#include <vector>
#include "../option_parser.h"
#include "../plugin.h"

#include "../symbolic/uniform_cost_search.h"
#include "../symbolic/sym_controller.h"

#include "symbolic_stackelberg_manager.h"

#include <cassert>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include "../utils/timer.h"

#include "follower_search_engine.h"
#include "follower_task.h"


using namespace std;
using namespace symbolic;

namespace stackelberg {

    SymbolicStackelberg::SymbolicStackelberg(const Options &opts) :
        SearchEngine(opts),
        optimal_engine(opts.get<FollowerSearchEngine *>("optimal_engine")),
        mgrParams(opts), searchParams(opts) {

        task = make_unique<StackelbergTask> ();
        stackelberg_mgr = make_shared<SymbolicStackelbergManager> (task.get(), opts);        
    }

    void SymbolicStackelberg::initialize() {
        cout << "Initializing SymbolicStackelberg..." << endl;
        auto t1 = chrono::high_resolution_clock::now();
        
        vars = stackelberg_mgr->get_sym_vars();

        leader_search_controller = make_unique<SymController> (vars, mgrParams, searchParams);
        leader_search = make_unique <UniformCostSearch> (leader_search_controller.get(), searchParams);
        auto mgr = stackelberg_mgr->get_leader_manager();
        
        leader_search->init(mgr, true);
        
        statistics.search_initialized(t1);
    }

    SearchStatus SymbolicStackelberg::step() {
        cout << "Starting Stackelberg bounded search..." << endl;

        int maxF = std::numeric_limits<int>::max();
        int L = 0;
        //int F = -1;

        const auto & closed_list = leader_search->getClosed()->getClosedList();
        while (!pareto_frontier.is_complete(maxF) && L < std::numeric_limits<int>::max()) {
            BDD leader_states = closed_list.at(L);
            BDD followerStates = stackelberg_mgr->get_follower_projection(leader_states);
            
            cout << "L = " << L << " leader states: " << vars->numStates(leader_states)
                 << "  follower subproblems: " << vars->numStates(followerStates, stackelberg_mgr->get_num_follower_bdd_vars()) << endl;


            //Generate the next layer
            while(!leader_search->finished() && leader_search->getG() == L) {
                leader_search->step();
            }

            assert (leader_search->getG() > L);
            L = leader_search->getG();   
        }



        cout << "Total number of leader states: " << vars->numStates(leader_search->getClosedTotal()) << endl;
        BDD followerStates = stackelberg_mgr->get_follower_projection(leader_search->getClosedTotal());
        cout << "Total number of follower subproblems: " << vars->numStates(followerStates, stackelberg_mgr->get_num_follower_bdd_vars()) << endl;
        cout << "Leader Search finished" << endl;
        exit(0);
        //cout << "Controller upper bound: " << controller->getUpperBound() << endl;
        //cout << "Desired bound: " << desired_bound << endl;

        return IN_PROGRESS;
    }
    
    SearchEngine *_parse_ss(OptionParser &parser) {
        SearchEngine::add_options_to_parser(parser);
        SymVariables::add_options_to_parser(parser);
        SymParamsSearch::add_options_to_parser(parser, 30e3, 10e7);
        SymParamsMgr::add_options_to_parser(parser);


        parser.add_option<FollowerSearchEngine *>("optimal_engine");
    
        Options opts = parser.parse();
        if (!parser.dry_run()) {
            return new stackelberg::SymbolicStackelberg(opts);
        }
        return NULL;
    }

    Plugin<SearchEngine> _plugin_sym_leader_search("sym_stackelberg", _parse_ss);
}

