#ifndef SECOND_ORDER_SEARCH_OPEN_LIST_H
#define SECOND_ORDER_SEARCH_OPEN_LIST_H

#include <cstdlib>

namespace second_order_search
{

class SearchNodeInfo;

template <typename Value>
class OpenList
{
public:
    virtual Value pop() = 0;
    virtual void push(const SearchNodeInfo &k, const Value &v) = 0;
    virtual size_t size() const = 0;
    virtual bool empty() const = 0;
};

}

#endif
