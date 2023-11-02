#include "pattern_collection_information.h"

#include "pattern_database.h"
#include "validation.h"

#include <algorithm>
#include <cassert>
#include <unordered_set>
#include <utility>

using namespace std;

namespace pdbs
{
PatternCollectionInformation::PatternCollectionInformation(
    shared_ptr<PatternCollection> patterns)
    :       patterns(patterns),
            pdbs(nullptr)
{
    assert(patterns);
    validate_and_normalize_patterns(*patterns);
}

bool PatternCollectionInformation::information_is_valid() const
{
    if (!patterns) {
        return false;
    }
    int num_patterns = patterns->size();
    if (pdbs) {
        if (patterns->size() != pdbs->size()) {
            return false;
        }
        for (int i = 0; i < num_patterns; ++i) {
            if ((*patterns)[i] != (*pdbs)[i]->get_pattern()) {
                return false;
            }
        }
    }
    return true;
}

void PatternCollectionInformation::create_pdbs_if_missing()
{
    assert(patterns);
    if (!pdbs) {
        pdbs = make_shared<PDBCollection>();
        for (const Pattern &pattern : *patterns) {
            shared_ptr<PatternDatabase> pdb =
                make_shared<PatternDatabase>(pattern);
            pdbs->push_back(pdb);
        }
    }
}

void PatternCollectionInformation::set_pdbs(shared_ptr<PDBCollection> pdbs_)
{
    pdbs = pdbs_;
    assert(information_is_valid());
}

shared_ptr<PatternCollection> PatternCollectionInformation::get_patterns()
{
    assert(patterns);
    return patterns;
}

shared_ptr<PDBCollection> PatternCollectionInformation::get_pdbs()
{
    create_pdbs_if_missing();
    return pdbs;
}
}
