
#include "open_list_factory.h"

#include "search_node_info.h"

namespace second_order_search
{

int GetGFromNode::operator()(const SearchNodeInfo &info) const
{
    return info.g;
}

std::pair<int, int> GetGRFromNode::operator()(const SearchNodeInfo &info) const
{
    return std::pair<int, int>(info.g, info.r);
}

void get_open_list_options(std::vector<std::string> &x)
{
    x.push_back("bgf");
    x.push_back("bgrf");
}


}
