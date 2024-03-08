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

using namespace std;
using namespace symbolic;

std::vector<std::vector<size_t>> comb(size_t N, size_t K)
{
    std::string bitmask(K, 1); // K leading 1's
    bitmask.resize(N, 0); // N-K trailing 0's

    std::vector<std::vector<size_t>> combinations;
    // print integers and permute bitmask
    do {
        std::vector<size_t> combination;
        for (size_t i = 0; i < N; ++i) // [0..N-1] integers
            if (bitmask[i])
                combination.push_back(i);
        combinations.push_back(std::move(combination));
    } while (std::prev_permutation(bitmask.begin(), bitmask.end()));

    return combinations;
}

vector<vector<int>> cartesian( vector<vector<int> >& v ) {
    auto product = []( long long a, vector<int>& b ) { return a*b.size(); };
    const long long N = accumulate( v.begin(), v.end(), 1LL, product );
    vector<vector<int>> combinations;
    vector<int> u(v.size());
    for( long long n=0 ; n<N ; ++n ) {
        lldiv_t q { n, 0 };
        for( long long i=v.size()-1 ; 0<=i ; --i ) {
            q = div( q.quot, v[i].size() );
            u[i] = v[i][q.rem];
        }

        combinations.push_back(u);
    }
    return combinations;
}
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
        cout << "Beginning state exploration" << endl;
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

        cout << "Finding relevant facts...";
        int fact_count = 0;
        std::unordered_map<int, std::vector<int>> variable_facts;
        std::vector<int> variables;
        for (int i = 0; i < g_variable_name.size(); i++) {
            std::vector<int> facts;
            for (int t = 0; t < g_fact_names[i].size(); t++) {
                if (g_fact_names[i][t].find("is-goal") != std::string::npos)
                    continue;
                if (g_fact_names[i][t].find("leader-state") != std::string::npos)
                    continue;
                if (g_fact_names[i][t].find("leader-turn") != std::string::npos)
                    continue;
                if (g_fact_names[i][t].find("none of those") != std::string::npos)
                    continue;
                fact_count++;
                facts.push_back(t);
            }
            if (!facts.empty()) {
                variable_facts[i] = facts;
                variables.push_back(i);
            }
        }
        cout << "Done" << endl;
        cout << "Task variables: " << variable_facts.size() << endl;
        cout << "Relevant variables: " << variable_facts.size() << endl;
        cout << "Task variable_facts: " << g_num_facts << endl;
        cout << "Relevant variable_facts: " << fact_count << endl;
        if (fact_count == 0) {
            cout << "No relevant facts" << endl;
            exit(0);
        }

        vector<vector<int>> variable_combinations;
        for (int i = 1; i <= 5; i++) {
            auto combinations = comb(variable_facts.size(), i);
            for (const auto &c : combinations) {
                vector<int> combination;
                for (const auto c_i : c)
                    combination.push_back(variables[c_i]);
                variable_combinations.push_back(combination);
            }
        }

        vector<pair<vector<pair<int, int>>, pair<int, int>>> result;
        for (const auto &v_com : variable_combinations) {
            vector<vector<int>> facts;
            for (const auto &i : v_com)
                facts.push_back(variable_facts.at(i));

            const auto product = cartesian(facts);
            for (const auto &p : product) {
                vector<pair<int, int>> v_facts;
                BDD applicable = bdd_valid | bdd_invalid;
                BDD invalid = bdd_invalid;
                for (int i = 0; i < v_com.size(); i++) {
                    applicable &= vars->preBDD(v_com[i], p[i]);
                    invalid &= vars->preBDD(v_com[i], p[i]);
                    v_facts.emplace_back(v_com[i], p[i]);
                }
                int applicable_count = vars->numStates(applicable);
                int invalid_count = vars->numStates(invalid);
                result.emplace_back(v_facts, std::make_pair(applicable_count, invalid_count));
            }
        }
        cout << "Writing to file...";
        std::ofstream plan_file("out");
        plan_file << vars->numStates(bdd_valid) << endl;
        plan_file << vars->numStates(bdd_invalid) << endl;
        for (const auto &r : result) {
            for (const auto &fact : r.first)
              plan_file  << g_fact_names[fact.first][fact.second] << '|';
            plan_file << endl;
            plan_file <<  r.second.first << endl;
            plan_file <<  r.second.second << endl;
        }
        plan_file.close();
        cout << "Done" << endl;

        exit(0);
    }
}
