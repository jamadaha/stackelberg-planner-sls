#ifndef SECOND_ORDER_SEARCH_H
#define SECOND_ORDER_SEARCH_H

#include "../search_engine.h"

#include "../timer.h"
#include "../int_packer.h"

#define MAX_REWARD std::numeric_limits<int>::max()

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
protected:
    size_t m_stat_inner_searches;
    Timer m_stat_time_inner_search;

    std::vector <const GlobalOperator *> m_available_operators;

    std::vector<bool> m_closed;

    IntPacker *m_counter_packer;
    std::vector<int> m_initial_state_counter;
    std::vector<std::vector<unsigned> > m_outer_to_inner_operator;
    std::vector<int> m_rewards;
    std::vector<std::vector<unsigned> > m_arcs;
    std::vector<std::vector<unsigned> > m_inv_arcs;

    void rgraph_exploration(const GlobalState &state,
                            std::vector<int> &res);

    int get_reward(const std::vector<int> &counter) const;
    int get_reward(const IntPacker::Bin *const &counter) const;
    int get_initial_state_reward(IntPacker::Bin *&result) const;
    int compute_reward_difference(const IntPacker::Bin *const &parent,
                                  const GlobalOperator &op,
                                  IntPacker::Bin *&res);

    void extract_inner_plan(const IntPacker::Bin *const &counter,
                            std::vector<const GlobalOperator *> &plan);

    virtual void initialize() override;
public:
    SecondOrderTaskSearch(const Options &opts);
    IntPacker *get_counter_packer() const
    {
        return m_counter_packer;
    }
    static void add_options_to_parser(OptionParser &parser);
};

}

#endif
