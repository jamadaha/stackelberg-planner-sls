#include "sym_bucket.h"

#include <algorithm>
#include <cassert>
#include <iostream>

using namespace std;

namespace symbolic {
void removeZero(Bucket &bucket) {
    bucket.erase(remove_if(std::begin(bucket), std::end(bucket),
                           [] (BDD &bdd) {return bdd.IsZero(); }),
                 std::end(bucket));
}

void copyBucket(const Bucket &bucket, Bucket &res) {
    if (!bucket.empty()) {
        res.insert(std::end(res), std::begin(bucket), std::end(bucket));
    }
}

void moveBucket(Bucket &bucket, Bucket &res) {
    copyBucket(bucket, res);
    Bucket().swap(bucket);
}

int nodeCount(const Bucket &bucket) {
    int sum = 0;
    for (const BDD &bdd : bucket) {
        sum += bdd.nodeCount();
    }
    return sum;
}


bool extract_states(Bucket &list,
                    const Bucket &pruned,
                    Bucket &res) {
    assert(!pruned.empty());

    bool somethingPruned = false;
    for (auto &bddList : list) {
        BDD prun = pruned[0] * bddList;

        for (const auto &prbdd :  pruned) {
            prun += prbdd * bddList;
        }

        if (!prun.IsZero()) {
            somethingPruned = true;
            bddList -= prun;
            res.push_back(prun);
        }
    }
    removeZero(list);
    return somethingPruned;
}


bool empty_intersection(const Bucket &list, const BDD & bdd) {
    for (const BDD & b : list) {
        if(!((b*bdd).IsZero())) {
            return false;
        }
    }
    return true;
}


    std::optional<BDD> get_non_empty_intersection(const Bucket &list, const BDD & bdd) {
        for (const BDD & b : list) {
            BDD res = b*bdd;
            if(!((res).IsZero())) {
                return std::make_optional<BDD>(res);
            }
        }
        return std::nullopt;
    }

    void insert_disjunctive(Bucket &list, const BDD & bdd, double limit_single, double limit_mult) {
        if (!list.empty() && list.back().nodeCount() < limit_single &&
            list.back().nodeCount() < limit_mult/bdd.nodeCount()) {
            list.back() += bdd;
        } else {
            list.push_back(bdd);
        }
        // for (auto & item : list) {
        //     cout << item.nodeCount() << " ";
        // }
        // cout << endl;    
    }

}
