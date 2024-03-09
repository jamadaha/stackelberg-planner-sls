#include "state_explorer.h"
#include "util.h"
#include "../option_parser.h"
#include "../plugin.h"
#include <vector>
#include "../symbolic/sym_controller.h"
#include "../symbolic/uniform_cost_search.h"
#include "symbolic_stackelberg_manager.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <sstream>
#include "follower_search_engine.h"
#include "plan_reuse.h"

using namespace std;
using namespace symbolic;

std::vector<std::vector<size_t>> comb(size_t N, size_t K)
{
    std::string bitmask(K, 1); // K leading 1's
    bitmask.resize(N, 0); // N-K trailing 0's

    std::vector<std::vector<size_t>> combinations;
    do {
        std::vector<size_t> combination;
        for (size_t i = 0; i < N; ++i) // [0..N-1] integers
            if (bitmask[i])
                combination.push_back(i);
        combinations.push_back(std::move(combination));
    } while (std::prev_permutation(bitmask.begin(), bitmask.end()));

    return combinations;
}

vector<vector<size_t>> cartesian( vector<vector<size_t> >& v ) {
    auto product = []( long long a, vector<size_t>& b ) { return a*b.size(); };
    const long long N = accumulate( v.begin(), v.end(), 1LL, product );
    vector<vector<size_t>> combinations;
    vector<size_t> u(v.size());
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

pair<string, vector<string>> split_operator(const string &s) {
    std::istringstream is(s);
    vector<string> objects;
    string name;
    string object;
    is >> std::skipws >> name;
    while (is >> std::skipws >> object) objects.push_back(object);
    return {name, objects};
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

    Plugin<SearchEngine> state_explore("state_explore", parse_ss);
}

namespace stackelberg {
    StateExplorer::StateExplorer(const Options &opts) :
    SearchEngine(opts),
    optimal_engine(opts.get<FollowerSearchEngine *>("optimal_engine")),
    plan_reuse(opts.get<PlanReuse *>("plan_reuse")), mgrParams(opts),
    searchParams(opts),
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

        for (const auto &o : g_operators) {
            std::string name = o.get_name();
            if (name.find('$') != string::npos) {
                this->parameter_count = std::count(name.begin(), name.end(), ' ');
                this->instantiations.push_back(split_operator(name).second);
            }
        }
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

        struct Fact {
            string name;
            vector<string> objects;
            size_t variable;
            size_t variable_val;

            explicit Fact(size_t variable, size_t value) : variable(variable), variable_val(value) {
                string s = g_fact_names[variable][value];
                size_t i = s.find('(');
                this->name = s.substr(0, i);
                s = s.substr(i + 1);
                while (s.size() > 1) {
                    if ((i = s.find(',')) != string::npos) {
                        this->objects.push_back(s.substr(0, i));
                        s = s.substr(i + 2);
                    } else if ((i = s.find(')') != string::npos)) {
                        this->objects.push_back(s.substr(0, i + 1));
                        s = s.substr(i + 2);
                    } else {
                        throw std::logic_error("Impossible!");
                    }
                }
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
                auto permutations = cartesian(indexes);
                for (auto &p: permutations)
                    lifted_precondition.emplace_back(predicate.first, std::move(p));
            }
        }
        cout << "Lifted preconditions: " << lifted_precondition.size() << endl;


        vector<vector<size_t>> combinations;
        for (size_t i = min_precondition_size; i <= max_precondition_size; i++) {
            auto c = comb(lifted_precondition.size(), i);
            combinations.insert(combinations.end(), c.begin(), c.end());
        }
        cout << "Combinations: " << combinations.size() << endl;

        for (const auto &o : g_operators)
            cout << o.get_name() << endl;

        unordered_map<size_t, pair<size_t, size_t>> result;
        for (size_t i = 0; i < combinations.size(); i++) {
            BDD applicable = bdd_valid | bdd_invalid;
            BDD invalid = bdd_invalid;
            for (const auto &c : combinations[i]) {
                for (const auto &f : facts) {
                    if (lifted_precondition[c].first != f.name)
                        continue;
                    bool valid = true;
                    for (const auto &instantiation : instantiations) {
                        if (!valid) break;
                        for (size_t t = 0; t < lifted_precondition[c].second.size(); t++)
                            if (instantiation[lifted_precondition[c].second[t]] != f.objects[t]) {
                                valid = false;
                                break;
                            }
                    }
                    if (!valid) continue;
                    applicable &= vars->preBDD(f.variable, f.variable_val);
                    invalid &= vars->preBDD(f.variable, f.variable_val);
                }
            }
            result[i] = {vars->numStates(applicable), vars->numStates(invalid)};
        }


        cout << "Writing to file...";
        std::ofstream plan_file("out");
        plan_file << vars->numStates(bdd_valid) << endl;
        plan_file << vars->numStates(bdd_invalid) << endl;
        for (const auto &r : result) {
            for (const auto &precondition : combinations[r.first])
              plan_file
              << lifted_precondition[precondition].first
              << lifted_precondition[precondition].second
              << '|';
            plan_file << endl;
            plan_file <<  r.second.first << endl;
            plan_file <<  r.second.second << endl;
        }
        plan_file.close();
        cout << "Done" << endl;

        exit(0);
    }
}
