#ifndef PDBS_PATTERN_COLLECTION_GENERATOR_ORDERED_SYSTEMATIC_H
#define PDBS_PATTERN_COLLECTION_GENERATOR_ORDERED_SYSTEMATIC_H

#include "pattern_generator.h"
#include "../option_parser.h"

namespace pdbs
{
class PatternCollectionGeneratorOrderedSystematic : public
    PatternCollectionGenerator
{
    size_t pattern_max_size;
public:
    explicit PatternCollectionGeneratorOrderedSystematic(const Options &opts);
    ~PatternCollectionGeneratorOrderedSystematic() = default;

    virtual PatternCollectionInformation generate(
        std::function<bool(const Pattern &)> handle_pattern = nullptr) override;

    static void add_options_to_parser(OptionParser &parser);
};
}

#endif
