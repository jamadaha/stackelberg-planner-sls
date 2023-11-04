#include "symbolic_leader_search.h"

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

using namespace std;
using namespace symbolic;

namespace stackelberg {

SymbolicStackelberg::SymbolicStackelberg(const Options &opts)
    : SearchEngine(opts),
      optimal_engine(opts.get<FollowerSearchEngine *>("optimal_engine")),
      plan_reuse(opts.get<PlanReuse *>("plan_reuse")), mgrParams(opts),
      searchParams(opts),
      upper_bound_pruning(opts.get<bool>("upper_bound_pruning")),
      statistics(opts.get<bool>("follower_info")),
      min_relevant_follower_cost(opts.get<int>("min_relevant_follower_cost")),
      replacement_dir(opts.get<std::string>("replacement_dir")),
      replacement_title(opts.get<std::string>("replacement_title")) {

  task = make_unique<StackelbergTask>();
  stackelberg_mgr = make_shared<SymbolicStackelbergManager>(task.get(), opts);
  optimal_engine->initialize(task.get(), stackelberg_mgr);

  plan_reuse->initialize(stackelberg_mgr);

  if (opts.contains("cost_bounded_engine")) {
    cost_bounded_engine.reset(
        opts.get<FollowerSearchEngine *>("cost_bounded_engine"));
    cost_bounded_engine->initialize(task.get(), stackelberg_mgr);
  }
}

void SymbolicStackelberg::initialize() {
  cout << "Initializing SymbolicStackelberg..." << endl;
  auto t1 = chrono::high_resolution_clock::now();

  vars = stackelberg_mgr->get_sym_vars();

  leader_search_controller =
      make_unique<SymController>(vars, mgrParams, searchParams);
  leader_search = make_unique<UniformCostSearch>(leader_search_controller.get(),
                                                 searchParams);
  auto mgr = stackelberg_mgr->get_leader_manager();

  leader_search->init(mgr, true);

  replacement_out = replacement_dir;
  if (replacement_title != ".") {
    replacement_out /= replacement_title;
  }
  std::filesystem::create_directories(replacement_out);

  statistics.stackelberg_search_initialized(t1);
}

SearchStatus SymbolicStackelberg::step() {
  cout << "Starting Stackelberg bounded search..." << endl;

  int maxF = std::numeric_limits<int>::max();
  int L = 0;
  int F = min_relevant_follower_cost;

  if (upper_bound_pruning) {
    cout << "Upper bound pruning:" << endl;
    auto t1 = chrono::high_resolution_clock::now();

    FollowerSolution minimum_ftask_solution =
        optimal_engine->solve_minimum_ftask(plan_reuse.get());
    auto runtime = chrono::duration_cast<chrono::milliseconds>(
        chrono::high_resolution_clock::now() - t1);
    statistics.follower_search_finished(runtime, true, minimum_ftask_solution);

    if (minimum_ftask_solution.is_solved()) {
      maxF = minimum_ftask_solution.solution_cost();
    }
  }

  const auto &closed_list = leader_search->getClosed()->getClosedList();
  while (!pareto_frontier.is_complete(maxF) &&
         L < std::numeric_limits<int>::max() && F < maxF) {
    BDD leader_states = closed_list.at(L);
    BDD follower_initial_states =
        stackelberg_mgr->get_follower_initial_state_projection(leader_states) *
        stackelberg_mgr->get_static_follower_initial_state();

    double num_follower_initial_states = vars->numStates(
        follower_initial_states, stackelberg_mgr->get_num_follower_bdd_vars());

    follower_initial_states =
        plan_reuse->find_plan_follower_initial_states(follower_initial_states);

    int newF = F;
    vector<int> current_best;
    FollowerSolution current_best_solution;
    while (!follower_initial_states.IsZero() && newF < maxF) {
      auto state = stackelberg_mgr->sample_follower_initial_state(
          follower_initial_states);

      FollowerSolution solution;
      if (cost_bounded_engine && newF > -1) {
        auto t1 = chrono::high_resolution_clock::now();
        solution = cost_bounded_engine->solve(state, plan_reuse.get(), newF);
        auto runtime = chrono::duration_cast<chrono::milliseconds>(
            chrono::high_resolution_clock::now() - t1);
        cout << " cb: " << runtime.count() / 1000.0;

        statistics.follower_search_finished(runtime, false, solution);
      }

      if (!solution.is_solved() ||
          (!solution.is_optimal() && solution.solution_cost() > newF)) {
        auto t1 = chrono::high_resolution_clock::now();

        solution = optimal_engine->solve(state, plan_reuse.get(),
                                         std::numeric_limits<int>::max());

        auto runtime = chrono::duration_cast<chrono::milliseconds>(
            chrono::high_resolution_clock::now() - t1);
        cout << " opt: " << runtime.count() / 1000.0 << endl;

        statistics.follower_search_finished(runtime, true, solution);
      }

      if (solution.has_plan() || solution.solution_cost() == 0) {
        auto plan = solution.get_plan();
        if (!plan.empty()) {
          std::ofstream plan_file(replacement_out / (get_uuid() + ".plan"));
          if (plan_file.is_open()) {
            vector<const GlobalOperator *> leader_ops_sequence;
            leader_search->getClosed()->extract_path(
                vars->getPartialStateBDD(
                    state,
                    stackelberg_mgr->get_pattern_vars_follower_subproblems()),
                L, true, leader_ops_sequence);
            plan_file << '(' << leader_ops_sequence.front()->get_name() << ')'
                      << endl;

            for (auto *op : plan)
              plan_file << '(' << op->get_name() << ')' << endl;
            plan_file.close();
          }
        }

        auto t1 = chrono::high_resolution_clock::now();

        follower_initial_states =
            plan_reuse->regress_plan_to_follower_initial_states(
                solution, follower_initial_states);
        auto runtime = chrono::duration_cast<chrono::milliseconds>(
            chrono::high_resolution_clock::now() - t1);

        cout << " reg: " << runtime.count() / 1000.0;
        // BDD aux =
        // vars->getPartialStateBDD(state,stackelberg_mgr->get_pattern_vars_follower_subproblems());
        // assert(follower_initial_states-aux !=follower_initial_states);
        // follower_initial_states -= aux;

      } else {
        assert(solution.solution_cost() == std::numeric_limits<int>::max());
      }
      int follower_cost = solution.solution_cost();

      cout << " solved: " << follower_cost << " total time: " << g_timer()
           << endl;

      if (follower_cost > newF) {
        newF = follower_cost;
        current_best = state;
        current_best_solution = solution;
        plan_reuse->set_follower_bound(newF);
      }
    }

    if (newF > F) {
      F = newF;

      // cout << "Extract leader path" << endl;
      // cout << "leader state: " << endl; for (size_t i =0; i <
      // g_fact_names.size(); ++i) { cout << g_fact_names[i][current_best[i]] <<
      // endl; }

      vector<const GlobalOperator *> leader_ops_sequence;
      leader_search->getClosed()->extract_path(
          vars->getPartialStateBDD(
              current_best,
              stackelberg_mgr->get_pattern_vars_follower_subproblems()),
          L, true, leader_ops_sequence);

      std::reverse(leader_ops_sequence.begin(), leader_ops_sequence.end());

      pareto_frontier.add_node(L, F, leader_ops_sequence,
                               current_best_solution.get_plan());
    }

    cout << "L = " << L << ", leader states: " << vars->numStates(leader_states)
         << ", follower subproblems: " << num_follower_initial_states << ", "
         << "F:" << F << ", total time: " << g_timer() << endl;

    // Generate the next layer
    while (!leader_search->finished() && leader_search->getG() == L) {
      leader_search->step();
    }

    assert(leader_search->getG() > L);
    L = leader_search->getG();
  }

  statistics.stackelberg_search_finished();
  BDD followerStates = stackelberg_mgr->get_follower_initial_state_projection(
                           leader_search->getClosedTotal()) *
                       stackelberg_mgr->get_static_follower_initial_state();
  cout << "Total number of leader states: "
       << vars->numStates(leader_search->getClosedTotal()) << endl;
  cout << "Total number of follower subproblems: "
       << vars->numStates(followerStates,
                          stackelberg_mgr->get_num_follower_bdd_vars())
       << endl;

  pareto_frontier.dump(*task);
  statistics.dump();

  cout << "Optimal engine:" << endl;
  optimal_engine->print_statistics();

  if (cost_bounded_engine) {
    cout << "Cost-bounded engine:" << endl;
    cost_bounded_engine->print_statistics();
  }

  exit(0);

  return IN_PROGRESS;
}

SearchEngine *_parse_ss(OptionParser &parser) {
  SearchEngine::add_options_to_parser(parser);
  SymbolicStackelbergManager::add_options_to_parser(parser);
  SymParamsSearch::add_options_to_parser(parser, 30e3, 10e7);

  parser.add_option<bool>("upper_bound_pruning", "Upper bound pruning.",
                          "true");
  parser.add_option<bool>("follower_info",
                          "Print additional info about follower searches",
                          "false");
  parser.add_option<int>(
      "min_relevant_follower_cost",
      "Any cost lower than this is irrelevant for the follower", "-1");

  parser.add_option<PlanReuse *>("plan_reuse", "strategy for plan reuse",
                                 "simple");
  parser.add_option<FollowerSearchEngine *>("optimal_engine");
  parser.add_option<FollowerSearchEngine *>("cost_bounded_engine", "", "",
                                            OptionFlags(false));

  // Used for replacement plans
  parser.add_option<std::string>(
      "replacement_dir",
      "Path to a directory in which follower plans are exported",
      "replacements", OptionFlags(false));

  parser.add_option<std::string>(
      "replacement_title",
      "What the directory replacements are added to should be called "
      "as a sub-directory of \"replacement_dir\"",
      ".", OptionFlags(false));

  Options opts = parser.parse();
  if (!parser.dry_run()) {
    return new stackelberg::SymbolicStackelberg(opts);
  }
  return NULL;
}

Plugin<SearchEngine> _plugin_sym_leader_search("sym_stackelberg", _parse_ss);
} // namespace stackelberg
