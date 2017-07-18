#include "second_order_task_search.h"

#include "globals.h"

#include "../option_parser.h"
#include "../globals.h"

#include "../state_registry.h"
#include "../successor_generator.h"

#include "../utilities.h"

#include "../delrax_search.h"

#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <deque>
#include <cstdio>
#include <fstream>
#include <unordered_set>

#include <cstring>

// #ifndef NDEBUG
// #define PRINT_STATE_SPACE
// #endif

// Assumptions for incremental rpg:
// 1) all attacker actions have at most one precondition on attacker variables
// 2) fix actions can only disable attacker actions

namespace second_order_search
{

SecondOrderTaskSearch::SecondOrderTaskSearch(const Options &opts)
    : SearchEngine(opts),
      c_silent(opts.get<bool>("silent")),
      c_incremental_rpg(opts.get<bool>("incremental_rpg")),
      m_inner_search(dynamic_cast<delrax_search::DelRaxSearch *>
                     (opts.get<SearchEngine *>("inner_search")))
{
}

void SecondOrderTaskSearch::initialize()
{
    preprocess_second_order_task();
    std::cout << "Initializing 2OT search ..." << std::endl;

    // *HACK* make sure inner search is properly initialized
    run_inner_search(g_outer_initial_state());

    if (!c_incremental_rpg) {
        return;
    }

    // copying data structures ...
    std::copy(m_inner_search->get_reward().begin(),
              m_inner_search->get_reward().end(),
              std::back_inserter(m_rewards));

    const std::vector<int> &pv = m_inner_search->get_positive_values();
    std::copy(pv.begin(), pv.end(), std::back_inserter(m_positive_values));
#ifndef NDEBUG
    for (size_t i = 0; i < pv.size(); i++) {
        assert(g_initial_state()[i] != pv[i]);
    }
#endif

    // TODO copy/compute goal i.e. facts with non-zero reward ?
    m_arcs.resize(g_variable_domain.size() + g_inner_operators.size());
    m_inv_arcs.resize(g_variable_domain.size() + g_inner_operators.size());

    g_outer_inner_successor_generator->generate_applicable_ops(
        g_outer_initial_state(), m_available_operators);

    for (unsigned i = 0; i < m_available_operators.size(); i++) {
        size_t op = m_available_operators[i]->get_op_id();

#ifndef NDEBUG
        for (const auto &p : g_inner_operator_outer_conditions[op]) {
            for (size_t oop = 0; oop < g_outer_operators.size(); oop++) {
                for (const auto &e : g_outer_operators[oop].get_effects()) {
                    if (p.var == e.var && p.val == e.val) {
                        std::cerr << "Error! "
                                  << g_outer_operators[oop].get_name()
                                  << " may enable "
                                  << g_inner_operators[op].get_name()
                                  << std::endl;
                    }
                    assert(p.var != e.var || p.val != e.val);
                }
            }
        }
        size_t pres = 0;
#endif

        for (const auto &p : g_inner_operators[op].get_preconditions()) {
            if (pv[p.var] == p.val) {
                assert(++pres == 1);
                m_arcs[p.var].push_back(g_variable_domain.size() + op);
                m_inv_arcs[g_variable_domain.size() + op].push_back(p.var);
            }
#ifndef NDEBUG
            else {
                assert(g_initial_state()[p.var] == p.val);
            }
#endif
        }

        for (const auto &e : g_inner_operators[op].get_effects()) {
            assert(pv[e.var] == e.val);
            m_arcs[g_variable_domain.size() + op].push_back(e.var);
            m_inv_arcs[e.var].push_back(g_variable_domain.size() + op);
        }
    }

    // computing outer to inner op map
    std::vector<std::vector<std::vector<unsigned> > > fact_neg_by(
        g_outer_variable_domain.size());
    for (unsigned var = 0; var < g_outer_variable_domain.size(); var++) {
        fact_neg_by[var].resize(g_outer_variable_domain[var]);
    }

    std::vector<int> prec(g_outer_variable_domain.size());
    for (unsigned opi = 0; opi < g_outer_operators.size(); opi++) {
        std::fill(prec.begin(), prec.end(), -1);
        const GlobalOperator &op = g_outer_operators[opi];
        for (const auto &p : op.get_preconditions()) {
            prec[p.var] = p.val;
        }
        for (const auto &e : op.get_effects()) {
            if (prec[e.var] == -1) {
                for (int val = 0; val < g_outer_variable_domain[e.var]; val++) {
                    if (val != e.val) {
                        fact_neg_by[e.var][val].push_back(opi);
                    }
                }
            } else {
                fact_neg_by[e.var][prec[e.var]].push_back(opi);
            }
        }
    }

    m_outer_to_inner_operator.resize(g_outer_operators.size());
    std::vector<bool> duplicate(g_outer_operators.size());
    assert(g_inner_operator_outer_conditions.size() == g_inner_operators.size());
    for (size_t i = 0; i < m_available_operators.size(); i++) {
        size_t opi = m_available_operators[i]->get_op_id();
        std::fill(duplicate.begin(), duplicate.end(), false);
        for (const auto &c : g_inner_operator_outer_conditions[opi]) {
            for (unsigned x : fact_neg_by[c.var][c.val]) {
                if (!duplicate[x]) {
                    duplicate[x] = true;
                    m_outer_to_inner_operator[x].push_back(g_variable_domain.size() + opi);
                    assert(x < m_outer_to_inner_operator.size());
                    assert(g_outer_operators[x].get_op_id() == (int) x);
                    assert(m_outer_to_inner_operator[x].back() < m_arcs.size());
                }
            }
        }
    }

#ifndef NDEBUG
    for (unsigned x = 0; x < m_arcs.size(); x++) {
        for (const unsigned &y : m_arcs[x]) {
            assert(x < g_variable_domain.size() || y < g_variable_domain.size());
            assert(x >= g_variable_domain.size() || y >= g_variable_domain.size());
        }
    }
    for (unsigned i = 0; i < m_available_operators.size(); i++) {
        unsigned nump = 0;
        for (const auto &c : m_available_operators[i]->get_preconditions()) {
            if (pv[c.var] == c.val) {
                nump++;
            }
        }
        assert(nump <= 1);
    }
    // verify acyclicity
    std::vector<bool> onstack;
    std::vector<int> ids(m_arcs.size(), -1);
    std::deque<unsigned> q;
    bool loop = false;
    std::vector<unsigned> loop_;
    for (unsigned x = 0; !loop && x < m_arcs.size(); x++) {
        if (ids[x] == -1) {
            q.push_back(x);
            while (!q.empty()) {
                unsigned y = q.back();
                q.pop_back();
                if (ids[y] == -1) {
                    if (loop) {
                        continue;
                    }
                    ids[y] = onstack.size();
                    onstack.push_back(true);
                }
                if (loop) {
                    loop_.push_back(y);
                    continue;
                }
                bool done = true;
                for (unsigned z : m_arcs[y]) {
                    assert(z < m_arcs.size());
                    if (ids[z] == -1) {
                        q.push_back(y);
                        q.push_back(z);
                        done = false;
                        break;
                    } else {
                        assert(ids[z] >= 0 && ids[z] < (int) onstack.size());
                        if (onstack[ids[z]]) {
                            loop = true;
                            std::cerr << "Cannot handle relaxed planning graphs with cycles!" << std::endl;
                            loop_.push_back(z);
                            loop_.push_back(y);
                            break;
                        }
                    }
                }
                if (done) {
                    onstack[ids[y]] = false;
                }
            }
        }
    }
    if (loop) {
        std::reverse(loop_.begin(), loop_.end());
        std::cerr << "---begin-cycle---" << std::endl;
        for (const unsigned &x : loop_) {
            if (x < g_variable_domain.size()) {
                std::cerr << g_fact_names[x][pv[x]] << std::endl;
            } else {
                std::cerr << g_inner_operators[x - g_variable_domain.size()].get_name() <<
                          std::endl;
            }
        }
        std::cerr << "---end-cycle---" << std::endl;
        exit_with(EXIT_CRITICAL_ERROR);
    }
#endif

    m_available_operators.clear();

    // computing counter ranges
    // the outer condition of all inner operators are satisfied in the initial
    // outer state
    rgraph_exploration(g_outer_initial_state(), m_initial_state_counter);

    std::vector<int> range(m_initial_state_counter);
    for (unsigned i = 0; i < range.size(); i++) {
        range[i] = std::max(2, range[i] + 1);
    }
    assert(range.size() == m_arcs.size());

    m_counter_packer = new IntPacker(range);

    m_closed.resize(g_variable_domain.size(), false);

    std::cout << "Bytes per RPG counter: " << (m_counter_packer->get_num_bins() *
              sizeof(IntPacker::Bin)) << std::endl;
}

void SecondOrderTaskSearch::rgraph_exploration(
    const GlobalState &state,
    std::vector<int> &res)
{
    // THIS FUNCTION MAY ONLY BE USED FOR THE INITIAL STATE!!!
    assert(state.get_id() == g_outer_initial_state().get_id());

    assert(m_available_operators.empty());

    res.clear();
    res.resize(m_arcs.size(), 0);

    std::deque<unsigned> open;

    g_outer_inner_successor_generator->generate_applicable_ops(
        state,
        m_available_operators);

    GlobalState inner_init = g_initial_state();
    for (unsigned i = 0; i < m_available_operators.size(); i++) {
        if (m_available_operators[i]->is_applicable(inner_init)) {
            size_t id = m_available_operators[i]->get_op_id() + g_variable_domain.size();
            assert(res[id] == 0);
            res[id] = 1;
            open.push_back(id);
        }
    }

    while (!open.empty()) {
        unsigned s = open.front();
        assert(s < g_variable_domain.size()
               || std::count(m_available_operators.begin(), m_available_operators.end(),
                             &g_inner_operators[s - g_variable_domain.size()]));
        open.pop_front();
        for (const unsigned &t : m_arcs[s]) {
            if (++res[t] == 1) {
                open.push_back(t);
            }
        }
    }

    m_available_operators.clear();
}


int SecondOrderTaskSearch::get_reward(const std::vector<int> &counter) const
{
    int res = 0;
    for (unsigned var = 0; var < g_variable_domain.size(); var++) {
        if (counter[var] > 0) {
            res += m_rewards[var];
        }
    }
    return -res;
}

int SecondOrderTaskSearch::get_reward(const IntPacker::Bin *const &counter)
const
{
    int res = 0;
    for (unsigned var = 0; var < g_variable_domain.size(); var++) {
        if (m_counter_packer->get(counter, var) > 0) {
            res += m_rewards[var];
        }
    }
    return -res;
}

int SecondOrderTaskSearch::get_initial_state_reward(IntPacker::Bin *&result)
const
{
    result = new IntPacker::Bin[m_counter_packer->get_num_bins()];
    for (unsigned i = 0; i < m_initial_state_counter.size(); i++) {
        m_counter_packer->set(result, i, m_initial_state_counter[i]);
    }
    return get_reward(result);
}

#ifndef NDEBUG
int SecondOrderTaskSearch::compute_reward_difference(
    const GlobalState &state,
    const IntPacker::Bin *const &parent,
    const GlobalOperator &op,
    IntPacker::Bin *&res)
#else
int SecondOrderTaskSearch::compute_reward_difference(
    const IntPacker::Bin *const &parent,
    const GlobalOperator &op,
    IntPacker::Bin *&res)
#endif
{
    res = new IntPacker::Bin[m_counter_packer->get_num_bins()];
    memcpy(res, parent, m_counter_packer->get_num_bins() * sizeof(IntPacker::Bin));

    std::deque<unsigned> open;
    assert(m_outer_to_inner_operator.size() == g_outer_operators.size());
    assert(op.get_op_id() < (int) m_outer_to_inner_operator.size());
    for (const unsigned &x : m_outer_to_inner_operator[op.get_op_id()]) {
        assert(x < m_arcs.size());
        if (m_counter_packer->get(res, x) > 0) {
            assert(x >= m_rewards.size());
            assert(m_counter_packer->get(res, x) == 1);
            m_counter_packer->set(res, x, 0);
            open.push_back(x);
        }
    }
    int diff = 0;
    int c;
    while (!open.empty()) {
        unsigned s = open.front();
        open.pop_front();
        for (const unsigned &t : m_arcs[s]) {
            c = m_counter_packer->get(res, t);
            if (c > 0) {
                c--;
                m_counter_packer->set(res, t, c);
                if (c == 0) {
                    open.push_back(t);
                    if (t < m_rewards.size()) {
                        diff += m_rewards[t];
                    }
                }
            }
        }
    }

#ifndef NDEBUG
    g_outer_inner_successor_generator->generate_applicable_ops(
        state,
        m_available_operators);
    for (size_t i = g_variable_domain.size(); i < m_arcs.size(); i++) {
        size_t opn = i - g_variable_domain.size();
        const GlobalOperator *op = &g_inner_operators[opn];
        assert(m_counter_packer->get(res, i) == 0
               || std::count(m_available_operators.begin(), m_available_operators.end(), op));
    }
    m_available_operators.clear();
#endif

    return -diff;
}

void SecondOrderTaskSearch::extract_inner_plan(const IntPacker::Bin *const
        &counter,
        std::vector<const GlobalOperator *> &plan)
{
    std::fill(m_closed.begin(), m_closed.end(), false);
    std::deque<unsigned> open;
    for (unsigned var = 0; var < m_closed.size(); var++) {
        if (m_rewards[var] > 0 && m_counter_packer->get(counter, var) > 0) {
            open.push_back(var);
            m_closed[var] = true;
        }
    }
    while (!open.empty()) {
        unsigned s = open.front();
        open.pop_front();
        for (const unsigned &o : m_inv_arcs[s]) {
            if (m_counter_packer->get(counter, o) > 0) {
                for (const unsigned &t : m_inv_arcs[o]) {
                    if (!m_closed[t]) {
                        open.push_back(t);
                        m_closed[t] = true;
                    }
                }
                plan.push_back(&g_inner_operators[o - g_variable_domain.size()]);
                break;
            }
        }
    }

    std::reverse(plan.begin(), plan.end());
}

int SecondOrderTaskSearch::run_inner_search(const GlobalState &state)
{
    g_outer_inner_successor_generator->generate_applicable_ops(
        state,
        m_available_operators);
    g_operators.clear();
    for (const GlobalOperator *op : m_available_operators) {
        g_operators.push_back(*op);
    }
    m_available_operators.clear();

    std::streambuf *old = std::cout.rdbuf(); // <-- save
    std::stringstream ss;
    std::cout.rdbuf(ss.rdbuf());        // <-- redirect
    m_inner_search->reset();
    m_inner_search->search();
    int res = MAX_REWARD;
    if (m_inner_search->found_solution()) {
        res = m_inner_search->calculate_plan_cost();
    }
    std::cout.rdbuf(old);   			// <-- restore

    return res;
}

bool SecondOrderTaskSearch::insert_into_pareto_frontier(int reward,
        int g,
        const StateID &state)
{
    typename ParetoFrontier::iterator it
        = m_pareto_frontier.lower_bound(reward);

    if ((it != m_pareto_frontier.end() && it->first == reward
            && it->second.first < g)
            || (it != m_pareto_frontier.begin()
                && ((--it)++)->second.first <= g)) {
        return false;
    }

    if (it != m_pareto_frontier.end() && it->first == reward) {
        if (it->second.first > g) {
            it->second.first = g;
            it->second.second.clear();
        }
        it->second.second.push_back(state);
    } else {
        std::pair<int, std::vector<StateID> > &entry =
            m_pareto_frontier[reward];
        entry.first = g;
        entry.second.push_back(state);
        it = m_pareto_frontier.lower_bound(reward);
    }

    assert(it != m_pareto_frontier.end());

    it++;
    while (it != m_pareto_frontier.end() && it->second.first >= g) {
        it = m_pareto_frontier.erase(it);
    }

    return true;
}

int SecondOrderTaskSearch::compute_max_reward()
{
    int res;
    IntPacker temp_packer(g_outer_variable_domain);
    PackedStateBin *buffer = new PackedStateBin[temp_packer.get_num_bins()];
    for (const auto &op : g_outer_operators) {
        for (const auto &e : op.get_effects()) {
            temp_packer.set(buffer, e.var, e.val);
        }
    }
    GlobalState best_state(buffer, NULL, StateID(std::numeric_limits<int>::max()),
                           &temp_packer);
    res = run_inner_search(best_state);
    delete[] buffer;
    return res;
}

void SecondOrderTaskSearch::set_inner_plan(IntPacker::Bin *counter)
{
    if (c_incremental_rpg) {
        extract_inner_plan(counter, g_plan);
    } else {
        assert(m_inner_search->found_solution());
        m_inner_search->save_plan_if_necessary();
    }
}


void SecondOrderTaskSearch::save_plan_if_necessary()
{
    std::cout << "(2OT) Pareto frontier consists of " << m_pareto_frontier.size() <<
              " groups" << std::endl;
    std::ostringstream json;
    json << "[";
    std::vector<std::vector<const GlobalOperator *> > paths;
    size_t num_states = 0;


    std::streambuf *old = NULL;
    if (c_silent) {
        old = std::cout.rdbuf(); // <-- save
        std::stringstream ss;
        std::cout.rdbuf(ss.rdbuf());        // <-- redirect
    }

    std::cout << "---begin-pareto-frontier---" << std::endl;
    unsigned num = 1;
    for (typename ParetoFrontier::reverse_iterator it = m_pareto_frontier.rbegin();
            it != m_pareto_frontier.rend();
            it++) {

        if (it != m_pareto_frontier.rbegin()) {
            json << ",\n";
        }
        json << "{"
             << "\"reward\": " << it->first
             << ", \"cost\": " << it->second.first
             << ", \"sequences\": [";

        std::cout << "    ---group-" << num << "--- {"
                  << "reward: " << it->first
                  << ", cost: " << it->second.first
                  << "}" << std::endl;

        size_t counter = 1;
        for (unsigned i = 0; i < it->second.second.size(); i++) {
#ifdef VERBOSE_DEBUGGING
            std::cout << "        ---begin-state-" << i << "--- [" <<
                      it->second.second[i].hash() << "]" << std::endl;
            GlobalState state = g_outer_state_registry->lookup_state(it->second.second[i]);
            for (unsigned var = 0; var < g_outer_variable_domain.size(); var++) {
                std::cout << "        " << g_outer_fact_names[var][state[var]] << std::endl;
            }
            std::cout << "        ---end-state---" << std::endl;
#endif

            get_paths(it->second.second[i], paths);
            for (const std::vector<const GlobalOperator *> &seq : paths) {
                if (counter > 1)  {
                    json << ",\n";
                }
                json << "  [";

                std::cout << "        " << "---sequence-" << counter << "---" << std::endl;
                if (seq.empty()) {
                    std::cout << "            <empty-sequence>" << std::endl;
                } else {
                    for (unsigned i = 0; i < seq.size(); i++) {
                        json << (i > 0 ? ", " : "")
                             << "\"" << seq[i]->get_name() << "\"";

                        std::cout << "            " << seq[i]->get_name();
#ifndef NDEBUG
                        std::cout << " (" << seq[i]->get_op_id() << ")";
#endif
                        std::cout << std::endl;
                    }
                }

                counter++;

                json << "]";
            }
            paths.clear();
            num_states++;
        }
        num++;

        json << "]}";
    }
    std::cout << "---end-pareto-frontier---" << std::endl;

    if (c_silent) {
        std::cout.rdbuf(old);
    }

    std::cout << "(2OT) state(s) in Pareto frontier: " << num_states << std::endl;


    json << "]";

    std::ofstream out;
    out.open("pareto_frontier.json");
    out << json.str();
    out.close();


#ifdef PRINT_STATE_SPACE
    std::ofstream of;
    of.open("state_space.dot");
    of << "digraph {" << std::endl;

    std::vector<const GlobalOperator *> aops;
    std::unordered_set<StateID> closed;
    std::deque<StateID> open;
    GlobalState init = g_outer_initial_state();
    of << "  state" << init.get_id().hash()
       << " [shape=ellipse, label=\"#" << init.get_id().hash() << "\", peripheries=2];"
       << std::endl;
    open.push_back(init.get_id());
    closed.insert(open.back());
    size_t slimit = g_outer_state_registry->size();
    size_t ec = 0;
    while (!open.empty()) {
        StateID sid = open.front();
        open.pop_front();
        GlobalState s = g_outer_state_registry->lookup_state(sid);
        g_outer_successor_generator->generate_applicable_ops(s, aops);
        for (size_t i = 0; i < aops.size(); i++) {
            GlobalState succ = g_outer_state_registry->get_successor_state(s, *aops[i]);
            if (succ.get_id().hash() < slimit) {
                if (!closed.count(succ.get_id())) {
                    of << "  state" << succ.get_id().hash() << " [shape=ellipse, label=\"#" <<
                       succ.get_id().hash()
                       << "\"";
                    if (in_pareto_frontier(succ.get_id())) {
                        of << ", peripheries=2";
                    }
                    of << "];" << std::endl;
                    closed.insert(succ.get_id());
                    open.push_back(succ.get_id());
                }
                of << "  edge" << ec << " [shape=rectangle, label=\""
                   << aops[i]->get_name() << "\"];" << std::endl;
                of << "  state" << sid.hash() << " -> edge" << ec << ";" << std::endl;
                of << "  edge" << ec << " -> state" << succ.get_id().hash()
                   << ";" << std::endl;
                ++ec;
            }
        }
        aops.clear();
    }

    of << "}" << std::endl;
    of.close();
#endif
}

bool SecondOrderTaskSearch::in_pareto_frontier(const StateID &state) const
{
    for (auto it = m_pareto_frontier.begin(); it != m_pareto_frontier.end(); it++) {
        for (auto init = it->second.second.begin(); init != it->second.second.end();
                init++) {
            if (state == *init) {
                return true;
            }
        }
    }
    return false;
}


void SecondOrderTaskSearch::add_options_to_parser(OptionParser &parser)
{
    parser.add_option<SearchEngine *>("inner_search", "", "delrax");
    parser.add_option<bool>("incremental_rpg", "", "true");
    parser.add_option<bool>("silent", "", "false");
    SearchEngine::add_options_to_parser(parser);
}

}
