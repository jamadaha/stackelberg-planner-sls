#ifndef PDBS_PATTERN_GENERATOR_H
#define PDBS_PATTERN_GENERATOR_H

#include "pattern_collection_information.h"
#include "types.h"

#include <functional>
#include <memory>

namespace pdbs
{
class PatternCollectionGenerator
{
public:
    virtual PatternCollectionInformation generate(
        std::function<bool(const Pattern &)> handle_pattern = nullptr) = 0;
};

class PatternGenerator
{
public:
    virtual Pattern generate() = 0;
};
}

#endif
