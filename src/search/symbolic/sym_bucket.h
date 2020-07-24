#ifndef SYMBOLIC_SYM_BUCKET_H
#define SYMBOLIC_SYM_BUCKET_H

#include <vector>
#include "cuddObj.hh"

namespace symbolic {
typedef std::vector<BDD> Bucket;

void removeZero(Bucket &bucket);
void copyBucket(const Bucket &bucket, Bucket &res);
void moveBucket(Bucket &bucket, Bucket &res);
int nodeCount(const Bucket &bucket);
bool extract_states(Bucket &list, const Bucket &pruned, Bucket &res);

bool empty_intersection(const Bucket &list, const BDD & bdd);
BDD get_non_empty_intersection(const Bucket &list, const BDD & bdd);
void insert_disjunctive(Bucket &list, const BDD & bdd, double limit_single = 20000.0, double limit_mult = 500000.0);
}

#endif
