#ifndef SECOND_ORDER_SEARCH_H
#define SECOND_ORDER_SEARCH_H

#include "../search_engine.h"

#include "../timer.h"
#include "../int_packer.h"

#include "../state_id.h"

#define MAX_REWARD 0 //std::numeric_limits<int>::max()

#include <map>

namespace delrax_search
{
class DelRaxSearch;
}

namespace second_order_search
{

template<typename C1, typename C2>
bool container_equals(const C1 &c1, const C2 &c2, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        if (c1[i] != c2[i]) {
            return false;
        }
    }
    return true;
}

struct RandomAccessBin {
    const IntPacker *packer;
    const IntPacker::Bin *bin;
    RandomAccessBin(const IntPacker *packer, const IntPacker::Bin *bin)
        : packer(packer), bin(bin) {}
    int operator[](size_t i) const
    {
        return packer->get(bin, i);
    }
};

class SecondOrderTaskSearch : public SearchEngine
{
private:
    std::vector <const GlobalOperator *> m_available_operators;
    std::vector<bool> m_closed;

    std::vector<int> m_initial_state_counter;
    std::vector<std::vector<unsigned> > m_outer_to_inner_operator;
    std::vector<int> m_rewards;
    std::vector<std::vector<unsigned> > m_arcs;
    std::vector<std::vector<unsigned> > m_inv_arcs;
    std::vector<int> m_positive_values;
protected:
    typedef std::map<int, std::pair<int, std::vector<StateID> >, std::greater<int> >
    ParetoFrontier;

    const bool c_silent;
    const bool c_incremental_rpg;

    ParetoFrontier m_pareto_frontier;

    delrax_search::DelRaxSearch *m_inner_search;
    IntPacker *m_counter_packer;

    void rgraph_exploration(const GlobalState &state,
                            std::vector<int> &res);

    int get_reward(const std::vector<int> &counter) const;
    int get_reward(const IntPacker::Bin *const &counter) const;
    int get_initial_state_reward(IntPacker::Bin *&result) const;
#ifndef NDEBUG
    int compute_reward_difference(const GlobalState &state,
                                  const IntPacker::Bin *const &parent,
                                  const GlobalOperator &op,
                                  IntPacker::Bin *&res);
#else
    int compute_reward_difference(const IntPacker::Bin *const &parent,
                                  const GlobalOperator &op,
                                  IntPacker::Bin *&res);
#endif

    void extract_inner_plan(const IntPacker::Bin *const &counter,
                            std::vector<const GlobalOperator *> &plan);

    int run_inner_search(const GlobalState &state);

    virtual void initialize() override;

    bool insert_into_pareto_frontier(int reward, int g, const StateID &state);
    virtual void get_paths(const StateID &state,
                           std::vector<std::vector<const GlobalOperator *> > &paths) = 0;
    int compute_max_reward();
    void set_inner_plan(IntPacker::Bin *counter);
    bool in_pareto_frontier(const StateID &state) const;
public:
    SecondOrderTaskSearch(const Options &opts);
    IntPacker *get_counter_packer() const
    {
        return m_counter_packer;
    }
    const std::vector<int> &get_inner_variable_rewards() const
    {
        return m_rewards;
    }
    const std::vector<int> &get_inner_positive_values() const
    {
        return m_positive_values;
    }
    virtual void save_plan_if_necessary() override;
    static void add_options_to_parser(OptionParser &parser);
};

}

#endif
