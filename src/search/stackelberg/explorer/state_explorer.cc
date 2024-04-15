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
            min_precondition_size(opts.get<size_t>("min_precondition_size")),
            max_precondition_size(opts.get<size_t>("max_precondition_size")),
            max_parameters(opts.get<size_t>("max_parameters")),
            exploration_time_limit(opts.get<size_t>("exploration_time_limit")) {
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
        cout << "Beginning precondition exploration" << endl;
        const string meta_name = MetaOperatorName();
        const auto default_instantiations = FindInstantiations(meta_name);
        cout << "Default default_instantiations: " << default_instantiations.size() << endl;
        const size_t default_parameters = ActionParameters(meta_name);
        const size_t desired_parameters = default_parameters + max_parameters;
        cout << "Default/desired parameters: " << default_parameters << '/' << desired_parameters << endl;
        cout << "Predicates: " << world.PredicateCount() << endl;

        vector<vector<size_t>> type_combs;
        for (size_t i = 0; i <= max_parameters; i++) {
            const auto combs = Cartesian(i, world.TypeCount());
            type_combs.insert(type_combs.end(), combs.begin(), combs.end());
        }
        cout << "Type combs: " << type_combs.size() << endl;

        size_t instantiation_count = 0;
        map<size_t, vector<vector<size_t>>> typed_instantiations;
        for (size_t i = 0; i < type_combs.size(); i++) {
            for (const auto &d_instantiation : default_instantiations) {
                vector<vector<size_t>> options;
                for (const auto &object : d_instantiation)
                    options.push_back({world.ObjectIndex(object)});
                for (unsigned int t : type_combs[i])
                    options.push_back(world.TypeObjects(t));
                for (const auto &instantiation : Cartesian(options)) {
                    typed_instantiations[i].push_back(instantiation);
                    instantiation_count++;
                }
            }
        }
        cout << "Instantiations: " << instantiation_count << endl;

        std::ofstream plan_file("out");
        plan_file << vars->numStates(valid | invalid) << endl;
        plan_file << vars->numStates(invalid) << endl;
        for (const auto &t_instantiations : typed_instantiations) {
            const auto &instantiations = t_instantiations.second;
            cout << "Parameters: " << instantiations.begin()->size() << endl;
            vector<pair<size_t, vector<size_t>>> preconditions;
            for (size_t i = 0; i < world.PredicateCount(); i++) {
                const size_t param_count = world.PredicateParameters(i);
                const auto permutations = Cartesian(param_count, instantiations.begin()->size());
                for (const auto &permutation : permutations)
                    preconditions.emplace_back(i, permutation);
            }
            vector<vector<size_t>> combinations;
            for (const auto &comb : Comb(preconditions.size(), min_precondition_size, max_precondition_size)) {
                vector<bool> usage(instantiations.begin()->size(), false);
                for (const auto c : comb)
                    for (const auto p : preconditions[c].second)
                        usage[p] = true;
                bool novel = true;
                for (size_t i = default_parameters; i < usage.size(); i++)
                    if (!usage[i])
                        novel = false;
                if (novel)
                    combinations.push_back(comb);
            }
            cout << "Combinations: " << combinations.size() << endl;

            for (const auto & comb : combinations) {
                BDD c_applicable = vars->zeroBDD();
                BDD c_invalid = vars->zeroBDD();

                for (const auto &instantiation : instantiations) {
                    BDD i_applicable = valid | invalid;
                    BDD i_invalid = invalid;

                    for (const auto &c : comb) {
                        const auto &pre = preconditions[c];
                        vector<size_t> objects;
                        for (const auto &p : pre.second) objects.push_back(instantiation[p]);
                        if (world.IsStatic(pre.first)) {
                            if (!world.HasStatic(pre.first, objects)) {
                                i_applicable = vars->zeroBDD();
                                i_invalid = vars->zeroBDD();
                            }
                            continue;
                        }
                        const auto f_bdd = world.FactBDD(pre.first, objects);
                        if (f_bdd == nullptr) {
                            i_applicable = vars->zeroBDD();
                            i_invalid = vars->zeroBDD();
                            break;
                        }
                        i_applicable &= *f_bdd;
                        i_invalid &= *f_bdd;
                        if (i_applicable == vars->zeroBDD()) break;
                    }

                    c_applicable |= i_applicable;
                    c_invalid |= i_invalid;
                }

                if (vars->numStates(c_applicable) == 0)
                    continue;

                for (const auto &t : type_combs[t_instantiations.first])
                    plan_file << world.TypeName(t) << ' ';
                plan_file << endl;
                for (const auto &p : comb)
                    plan_file << world.PredicateName(preconditions[p].first) << preconditions[p].second << '|';
                plan_file << endl;
                plan_file << vars->numStates(c_applicable) << endl;
                plan_file << vars->numStates(c_invalid) << endl;
            }
        }
        plan_file.close();

        exit(0);
    }
}
