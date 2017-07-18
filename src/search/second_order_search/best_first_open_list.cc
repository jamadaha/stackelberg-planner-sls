
#ifdef SECOND_ORDER_SEARCH_BEST_FIRST_OPEN_LIST_H

#include <cassert>

namespace second_order_search
{

template<typename Key, typename Value, typename Compare, typename GetKey>
BestFirstOpenList<Key, Value, Compare, GetKey>::BestFirstOpenList(
    const Compare &compare)
    : m_open_list(compare),
      m_size(0)
{}

template<typename Key, typename Value, typename Compare, typename GetKey>
Value BestFirstOpenList<Key, Value, Compare, GetKey>::pop()
{
    assert(!empty());
    typename t_open_list::iterator it = m_open_list.begin();
    Value res = it->second.front();
    it->second.pop_front();
    if (it->second.empty()) {
        m_open_list.erase(it);
    }
    m_size--;
    return res;
}

template<typename Key, typename Value, typename Compare, typename GetKey>
void BestFirstOpenList<Key, Value, Compare, GetKey>::push(
    const SearchNodeInfo &sn,
    const Value &v)
{
    Key key = m_get_key(sn);
    std::deque<Value> &l = m_open_list[key];
    l.push_back(v);
    m_size++;
}

template<typename Key, typename Value, typename Compare, typename GetKey>
size_t BestFirstOpenList<Key, Value, Compare, GetKey>::size() const
{
    return m_size;
}

template<typename Key, typename Value, typename Compare, typename GetKey>
bool BestFirstOpenList<Key, Value, Compare, GetKey>::empty() const
{
    return m_size == 0;
}

}

#endif
