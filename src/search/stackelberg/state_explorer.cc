#include "state_explorer.h"
#include "util.h"
#include "../option_parser.h"
#include "../plugin.h"
#include <filesystem>
#include <vector>
#include "../symbolic/sym_controller.h"
#include "../symbolic/uniform_cost_search.h"
#include "symbolic_stackelberg_manager.h"
#include "../utils/timer.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include "follower_search_engine.h"
#include "follower_task.h"
#include "plan_reuse.h"
#include "../symbolic/sym_variables.h"
#include "bdd_tree.h"

using namespace std;
using namespace symbolic;

namespace {
    SearchEngine *parse_ss(OptionParser &parser) {
        SearchEngine::add_options_to_parser(parser);
        stackelberg::SymbolicStackelbergManager::add_options_to_parser(parser);
        SymParamsSearch::add_options_to_parser(parser, 30e3, 10e7);

        parser.add_option<bool>("upper_bound_pruning", "Upper bound pruning.",
                                "true");
        parser.add_option<bool>("follower_info",
                                "Print additional info about follower searches",
                                "false");
        parser.add_option<int>(
                "min_relevant_follower_cost",
                "Any cost lower than this is irrelevant for the follower", "-1");

        parser.add_option<stackelberg::PlanReuse *>("plan_reuse", "strategy for plan reuse",
                                       "simple");
        parser.add_option<stackelberg::FollowerSearchEngine *>("optimal_engine");
        parser.add_option<stackelberg::FollowerSearchEngine *>("cost_bounded_engine", "", "",
                                                  OptionFlags(false));

        Options opts = parser.parse();
        if (!parser.dry_run()) {
            return new stackelberg::StateExplorer(opts);
        }
        return NULL;
    }

    Plugin<SearchEngine> state_explore("state_explore", parse_ss);
}

namespace stackelberg {
    StateExplorer::StateExplorer(const Options &opts) :
    SearchEngine(opts),
    optimal_engine(opts.get<FollowerSearchEngine *>("optimal_engine")),
    plan_reuse(opts.get<PlanReuse *>("plan_reuse")), mgrParams(opts),
    searchParams(opts),
    upper_bound_pruning(opts.get<bool>("upper_bound_pruning")),
    statistics(opts.get<bool>("follower_info")),
    min_relevant_follower_cost(opts.get<int>("min_relevant_follower_cost")) {
        task = make_unique<StackelbergTask>();
        stackelberg_mgr = std::make_shared<SymbolicStackelbergManager>(task.get(), opts);
        optimal_engine->initialize(task.get(), stackelberg_mgr);

        plan_reuse->initialize(stackelberg_mgr);

        if (opts.contains("cost_bounded_engine")) {
            cost_bounded_engine.reset(
                    opts.get<FollowerSearchEngine *>("cost_bounded_engine"));
            cost_bounded_engine->initialize(task.get(), stackelberg_mgr);
        }
    }

    void StateExplorer::initialize() {
        cout << "Initializing State Explore..." << endl;
        auto t1 = chrono::high_resolution_clock::now();

        vars = stackelberg_mgr->get_sym_vars();

        leader_search_controller =
                make_unique<SymController>(vars, mgrParams, searchParams);
        leader_search = make_unique<UniformCostSearch>(leader_search_controller.get(),
                                                       searchParams);
        auto mgr = stackelberg_mgr->get_leader_manager();

        leader_search->init(mgr, true);

        statistics.stackelberg_search_initialized(t1);
    }

    SearchStatus StateExplorer::step() {
      BDD bdd_valid = vars->zeroBDD();
      BDD bdd_invalid = vars->zeroBDD();
      const auto &closed_list = leader_search->getClosed()->getClosedList();
      for (int L = 0; L < std::numeric_limits<int>::max();) {
        if (closed_list.find(L) == closed_list.end())
          break;
        cout << "L: " << L << endl;
      auto leader_states = closed_list.at(L);
      BDD follower_initial_states =
              stackelberg_mgr->get_follower_initial_state_projection(leader_states) *
              stackelberg_mgr->get_static_follower_initial_state();

      follower_initial_states =
              plan_reuse->find_plan_follower_initial_states(follower_initial_states);
      size_t state_count = 0;
      // TODO: What is a good way of limiting this loop?
      while (!follower_initial_states.IsZero()  ) {
        // TODO: Is sample random?
        auto state = stackelberg_mgr->sample_follower_initial_state(
                follower_initial_states);
        auto solution = optimal_engine->solve(state, plan_reuse.get(),
                                              std::numeric_limits<int>::max());
        if (solution.has_plan() || solution.solution_cost() == 0) {
          bdd_valid += vars->getStateBDD(state);
          follower_initial_states =
                  plan_reuse->regress_plan_to_follower_initial_states(
                          solution, follower_initial_states);
        } else {
          bdd_invalid += vars->getStateBDD(state);
          BDD aux =
                  vars->getPartialStateBDD(state, stackelberg_mgr->get_pattern_vars_follower_subproblems());
          assert(follower_initial_states - aux != follower_initial_states);
          follower_initial_states -= aux;
        }

      }

        cout << "Valid: " << vars->numStates(bdd_valid) << endl;
        cout << "Invalid: " << vars->numStates(bdd_invalid) << endl;

      cout << "Generating next layer...";
      while (!leader_search->finished() && leader_search->getG() == L) {
        leader_search->step();
      }
      cout << "Done" << endl;

      assert(leader_search->getG() > L);
      L = leader_search->getG();

    }




        if (vars->numStates(bdd_invalid) == 0)
            exit(0);

        BDDTree<std::pair<size_t, size_t>> tree = BDDTree<std::pair<size_t, size_t>>(vars);
        for (size_t i = 0; i < g_variable_name.size(); i++) {
            for (size_t t = 0; t < g_fact_names[i].size(); t++) {
                if (g_fact_names[i][t].find("is-goal") != std::string::npos)
                    continue;
                if (g_fact_names[i][t].find("leader-state") != std::string::npos)
                    continue;
                if (g_fact_names[i][t].find("leader-turn") != std::string::npos)
                    continue;
                if (g_fact_names[i][t].find("none of those") != std::string::npos)
                    continue;
                const BDD bdd = bdd_invalid & (~vars->preBDD(i, t));
                tree.AddRoot({i, t}, bdd);
            }
        }
        cout << "Root size: " << tree.RootSize() << endl;
        auto result = tree.Generate();
        cout << "Result size: " << result.size() << endl;

        std::ofstream plan_file("out");
        plan_file << vars->numStates(bdd_valid) << endl;
        plan_file << vars->numStates(bdd_invalid) << endl;
        for (const auto &r : result) {
            for (const auto &fact : r.first) {
              plan_file  << g_fact_names[fact.first][fact.second] << '|';
            }
            plan_file << endl;
            plan_file <<  r.second << endl;
        }
        plan_file.close();

        exit(0);
    }

}
