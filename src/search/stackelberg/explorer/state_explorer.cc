#include "state_explorer.h"
#include "util.h"
#include "../../option_parser.h"
#include "../../plugin.h"
#include <vector>
#include "../../symbolic/sym_controller.h"
#include "../../symbolic/uniform_cost_search.h"
#include "../symbolic_stackelberg_manager.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <sstream>
#include "../follower_search_engine.h"
#include "../plan_reuse.h"
#include <chrono>

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
      parser.add_option<vector<string>>(
                "type_names",
                "", "[]");
      parser.add_option<vector<vector<string>>>(
                "type_objects",
                "", "[]");
      parser.add_option<vector<string>>(
              "static_names",
              "", "[]");
      parser.add_option<vector<vector<vector<string>>>>(
                "static_facts",
                "", "[]");
      parser.add_option<size_t>(
              "min_precondition_size",
              "Minimum number of facts added to precondition", "1");
      parser.add_option<size_t>(
              "max_precondition_size",
              "Maximum number of facts added to precondition", "2");
      parser.add_option<size_t>(
                "min_parameters",
                "", "0");
      parser.add_option<size_t>(
                "max_parameters",
                "", "0");
      parser.add_option<size_t>(
                "exploration_time_limit",
                "Time allowed for state exploration in seconds", "10000"
      );

      Options opts = parser.parse();
      if (!parser.dry_run()) {
        return new stackelberg::StateExplorer(opts);
      }
      return NULL;
    }

    Plugin<SearchEngine> state_explore("state_explorer", parse_ss);
}

namespace stackelberg {
    StateExplorer::StateExplorer(const Options &opts) :
            SearchEngine(opts),
            optimal_engine(opts.get<FollowerSearchEngine *>("optimal_engine")),
            plan_reuse(opts.get<PlanReuse *>("plan_reuse")), mgrParams(opts),
            searchParams(opts),
            world(
                    opts.get<vector<string>>("type_names"),
                    opts.get<vector<vector<string>>>("type_objects"),
                    opts.get<vector<string>>("static_names"),
                    opts.get<vector<vector<vector<string>>>>("static_facts")
                    ),
            exploration_time_limit(opts.get<size_t>("exploration_time_limit")){
      combiner = make_unique<Combiner>(
        opts.get<size_t>("min_parameters"),
        opts.get<size_t>("max_parameters"),
        opts.get<size_t>("min_precondition_size"),
        opts.get<size_t>("max_precondition_size")
      );
      task = make_unique<StackelbergTask>();
      stackelberg_mgr = std::make_shared<SymbolicStackelbergManager>(task.get(), opts);
      optimal_engine->initialize(task.get(), stackelberg_mgr);
      plan_reuse->initialize(stackelberg_mgr);
    }

    std::pair<BDD, BDD> StateExplorer::explore() {
        const std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
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
            while (!follower_initial_states.IsZero()) {
                const std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
                const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(t1 - t0).count();
                if (elapsed > exploration_time_limit) goto TIME_OUT;

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
                    BDD bdd_state = vars->getPartialStateBDD(state,
                                                             stackelberg_mgr->get_pattern_vars_follower_subproblems());
                    follower_initial_states -= bdd_state;
                }
            }
            cout << "Valid: " << vars->numStates(bdd_valid) << endl;
            cout << "Invalid: " << vars->numStates(bdd_invalid) << endl;

            cout << "Generating next layer...";
            while (!leader_search->finished() && leader_search->getG() == L) {
                const std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
                const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(t1 - t0).count();
                if (elapsed > exploration_time_limit) goto TIME_OUT;
                leader_search->step();
            }
            cout << "Done" << endl;

            assert(leader_search->getG() > L);
            L = leader_search->getG();
        }
TIME_OUT:
        return {bdd_valid, bdd_invalid};
    }

    void StateExplorer::initialize() {
      cout << "Initializing State Explore..." << endl;
      vars = stackelberg_mgr->get_sym_vars();
      leader_search_controller =
              make_unique<SymController>(vars, mgrParams, searchParams);
      leader_search = make_unique<UniformCostSearch>(leader_search_controller.get(),
                                                     searchParams);
      auto mgr = stackelberg_mgr->get_leader_manager();
      leader_search->init(mgr, true);
      world.Init(vars);
      combiner->Init(vars, world);
    }

    SearchStatus StateExplorer::step() {
        cout << "Beginning state exploration" << endl;
        const pair<BDD, BDD> validity = explore();
        const BDD valid = validity.first;
        const BDD invalid = validity.second;
        if (invalid.IsZero()) {
            cout << "Meta action is valid" << endl;
            exit(0);
        }

        std::ofstream plan_file("out");
        plan_file << vars->numStates(valid | invalid) << endl;
        plan_file << vars->numStates(invalid) << endl;
        combiner->Combine(
            this->world,
            valid,
            invalid,
            [&](const auto &combination){
                for (const auto & p : combination.params)
                    plan_file << world.TypeName(p) << ' ';
                plan_file << endl;
                for (const auto &l : combination.literals) {
                    plan_file << world.PredicateName(l.predicate);
                    for (const auto &p : l.params)
                        plan_file << ' ' << p;
                    plan_file << '|';
                }
                plan_file << endl;
                plan_file << combination.applicable << endl;
                plan_file << combination.invalid << endl;
            }
        );
        plan_file.close();

        exit(0);
    }
}
