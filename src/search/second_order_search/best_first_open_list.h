#ifndef SECOND_ORDER_SEARCH_BEST_FIRST_OPEN_LIST_H
#define SECOND_ORDER_SEARCH_BEST_FIRST_OPEN_LIST_H

#include "open_list.h"

#include <map>
#include <deque>

namespace second_order_search
{

template<typename Key, typename Value, typename Compare, typename GetKey>
class BestFirstOpenList : public OpenList<Value>
{
    typedef typename std::map<Key, std::deque<Value>, Compare> t_open_list;
    GetKey m_get_key;
    t_open_list m_open_list;
    size_t m_size;
public:
    BestFirstOpenList(const Compare &compare = Compare());
    virtual void push(const SearchNodeInfo &k, const Value &v) override;
    virtual Value pop() override;
    virtual size_t size() const override;
    virtual bool empty() const override;
};

}

#include "best_first_open_list.cc"

#endif
