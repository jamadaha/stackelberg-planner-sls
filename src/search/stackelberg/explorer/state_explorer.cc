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
              "statics",
              "", "[]");
      parser.add_option<size_t>(
              "min_precondition_size",
              "Minimum number of facts added to precondition", "1");
      parser.add_option<size_t>(
              "max_precondition_size",
              "Maximum number of facts added to precondition", "4");

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
            world(opts.get<vector<string>>("statics")),
            min_precondition_size(opts.get<size_t>("min_precondition_size")),
            max_precondition_size(opts.get<size_t>("max_precondition_size")) {
      task = make_unique<StackelbergTask>();
      stackelberg_mgr = std::make_shared<SymbolicStackelbergManager>(task.get(), opts);
      optimal_engine->initialize(task.get(), stackelberg_mgr);
      plan_reuse->initialize(stackelberg_mgr);
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
    }

    SearchStatus StateExplorer::step() {
      cout << "Beginning state exploration" << endl;
      const auto meta_name = MetaOperatorName();
      const auto parameter_count = ActionParameters(meta_name);
      const auto instantiations = FindInstantiations(meta_name);
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
            BDD bdd_state = vars->getPartialStateBDD(state, stackelberg_mgr->get_pattern_vars_follower_subproblems());
            follower_initial_states -= bdd_state;
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
      cout << "Finished state exploration" << endl;
      if (vars->numStates(bdd_invalid) == 0) {
        cout << "Meta action is valid" << endl;
        exit(0);
      }

      struct Fact {
          string name;
          vector<string> objects;
          size_t variable;
          size_t variable_val;

          explicit Fact(size_t variable, size_t value) : variable(variable), variable_val(value) {
            string s = g_fact_names[variable][value];
            const auto p = SplitFact(s);
            this->name = p.first;
            this->objects = p.second;
          }
      };

      vector<Fact> facts;
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
          facts.emplace_back(i, t);
        }
      }
      cout << "Facts: " << facts.size() << endl;

      vector<pair<string, size_t>> predicates;
      for (const auto &fact : facts) {
        bool found = false;
        for (const auto &p: predicates)
          if (fact.name == get<0>(p))
            found = true;
        if (found) continue;
        predicates.emplace_back(fact.name, fact.objects.size());
      }
      cout << "Predicates: " << predicates.size() << endl;

      vector<pair<string, vector<size_t>>> lifted_precondition;
      for (const auto &predicate : predicates) {
        vector<vector<size_t>> indexes;
        for (size_t i = 0; i < predicate.second; i++) {
          vector<size_t> v;
          for (size_t t = 0; t < parameter_count; t++)
            v.push_back(t);
          indexes.push_back(v);
        }
        if (indexes.empty()) {
          lifted_precondition.emplace_back(predicate.first, vector<size_t>());
        } else {
          auto permutations = Cartesian(indexes);
          for (auto &p: permutations)
            lifted_precondition.emplace_back(predicate.first, std::move(p));
        }
      }
      cout << "Lifted preconditions: " << lifted_precondition.size() << endl;


      vector<vector<size_t>> combinations;
      for (size_t i = min_precondition_size; i <= max_precondition_size; i++) {
        auto c = Comb(lifted_precondition.size(), i);
        combinations.insert(combinations.end(), c.begin(), c.end());
      }
      cout << "Combinations: " << combinations.size() << endl;

      vector<tuple<size_t, size_t, size_t>> result;
      for (size_t i = 0; i < combinations.size(); i++) {
        BDD applicable = vars->zeroBDD();
        BDD invalid = vars->zeroBDD();
        for (const auto &instantiation : instantiations) {
          BDD i_applicable = bdd_valid | bdd_invalid;
          BDD i_invalid = bdd_invalid;
          for (const auto &c : combinations[i]) {
            const auto &l_p = lifted_precondition[c];
            // map precondition to instantiation objects
            vector<string> objects;
            for (const auto &parameter_index : l_p.second)
              objects.push_back(instantiation[parameter_index]);
            // find fact that matches predicate & objects
            Fact const* fact = nullptr;
            for (const auto &f : facts)
              if (f.name == l_p.first && f.objects == objects) {
                fact = &f;
                break;
              }
            if (fact == nullptr){
              // If no fact exists for the permutation, there is no state wherein it is applicable
              // This is usually because it somehow breaks types
              i_applicable &= vars->zeroBDD();
              i_invalid &= vars->zeroBDD();
              break;
            }
            i_applicable &= vars->preBDD(fact->variable, fact->variable_val);
            i_invalid &= vars->preBDD(fact->variable, fact->variable_val);
          }
          applicable |= i_applicable;
          invalid |= i_invalid;
        }

        if (applicable == (bdd_valid | bdd_invalid)) continue;

        result.emplace_back(i, vars->numStates(applicable), vars->numStates(invalid));
      }

      sort(result.begin(), result.end(), [](const auto &lhs, const auto &rhs) {
          if (get<1>(lhs) == 0) return false;
          if (get<1>(rhs) == 0) return true;
          return get<1>(lhs) < get<1>(rhs);
      });

      cout << "Result: " << result.size() << endl;
      cout << "Writing to file...";
      std::ofstream plan_file("out");
      plan_file << vars->numStates(bdd_valid) << endl;
      plan_file << vars->numStates(bdd_invalid) << endl;
      for (const auto &r : result) {
        for (const auto &precondition : combinations[get<0>(r)])
          plan_file
                  << lifted_precondition[precondition].first
                  << lifted_precondition[precondition].second
                  << '|';
        plan_file << endl;
        plan_file <<  get<1>(r) << endl;
        plan_file <<  get<2>(r) << endl;
      }
      plan_file.close();
      cout << "Done" << endl;

      exit(0);
    }
}