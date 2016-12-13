/*
 * fixactionssearch.h
 *
 *  Created on: 12.12.2016
 *      Author: Patrick
 */

#ifndef SRC_SEARCH_FIXACTIONS_SEARCH_H_
#define SRC_SEARCH_FIXACTIONS_SEARCH_H_

#include "search_engine.h"

class FixActionsSearch: public SearchEngine {

protected:
    virtual void initialize();
    virtual SearchStatus step();

public:
	FixActionsSearch(const Options &opts);
	virtual ~FixActionsSearch();
	virtual void statistics() const;
};

#endif /* SRC_SEARCH_FIXACTIONS_SEARCH_H_ */
