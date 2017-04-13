#ifndef SECOND_ORDER_SEARCH_OPEN_LIST_FACTORY_H
#define SECOND_ORDER_SEARCH_OPEN_LIST_FACTORY_H

#include "open_list.h"
#include "best_first_open_list.h"

#include "../option_parser.h"

#include <functional>
#include <vector>
#include <string>

namespace second_order_search
{

class SearchNodeInfo;

enum OPEN_LIST {
    BGF = 0, // Best G First
    BGRF = 1, // Best G First with R tiebreaking
};

void get_open_list_options(std::vector<std::string> &x);
template<typename Value>
OpenList<Value> *parse_open_list(OptionParser &parser);

struct GetGFromNode {
    int operator()(const SearchNodeInfo &info) const;
};

struct GetGRFromNode {
    std::pair<int, int> operator()(const SearchNodeInfo &info) const;
};

struct CompareGR {
    bool operator()(const std::pair<int, int> &x,
                    const std::pair<int, int> &y) const
    {
        return x.first < y.first || (x.first == y.first && x.second > y.second);
    }
};

template<typename Value>
OpenList<Value> *parse_open_list(int open_list)
{
    switch (OPEN_LIST(open_list)) {
    case BGF:
        return
            new BestFirstOpenList<int, Value, std::less<int>, GetGFromNode>();
    case BGRF:
        return
            new BestFirstOpenList<std::pair<int, int>, Value, CompareGR, GetGRFromNode>();
    }
    return NULL;
}

}

#endif
