#ifndef ACYCLIC_DELETE_FREE_SEARCH_H
#define ACYCLIC_DELETE_FREE_SEARCH_H

#include "../search_engine.h"

namespace second_order_search
{

class DeleteFreeTaskSearch : public SearchEngine
{
protected:
    virtual void initialize();
    virtual SearchStatus step();
public:
    DeleteFreeTaskSearch(const Options &opts);
    static void add_options_to_parser(OptionParser &parser);
};

}

#endif
