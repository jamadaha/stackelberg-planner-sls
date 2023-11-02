#ifndef PDBS_PATTERN_COLLECTION_GENERATOR_SYSTEMATIC_H
#define PDBS_PATTERN_COLLECTION_GENERATOR_SYSTEMATIC_H

#include "pattern_generator.h"
#include "types.h"
#include "../utils/hash.h"
#include "../option_parser.h"

#include <cstdlib>
#include <memory>
#include <unordered_set>
#include <vector>

class CausalGraph;

namespace pdbs
{
// Invariant: patterns are always sorted.
class PatternCollectionGeneratorSystematic : public PatternCollectionGenerator
{
    using PatternSet = std::unordered_set<Pattern>;

    const size_t max_pattern_size;
    const bool only_interesting_patterns;
    std::shared_ptr<PatternCollection> patterns;
    PatternSet pattern_set;  // Cleared after pattern computation.

    bool enqueue_pattern_if_new(const Pattern &pattern,
                                std::function<bool(const Pattern &)> handle_pattern);
    void compute_eff_pre_neighbors(const CausalGraph &cg,
                                   const Pattern &pattern,
                                   std::vector<int> &result) const;
    void compute_connection_points(const CausalGraph &cg,
                                   const Pattern &pattern,
                                   std::vector<int> &result) const;

    bool build_sga_patterns(const CausalGraph &cg,
                            std::function<bool(const Pattern &)> handle_pattern);
    void build_patterns(std::function<bool(const Pattern &)> handle_pattern);
    void build_patterns_naive(std::function<bool(const Pattern &)> handle_pattern);
public:
    explicit PatternCollectionGeneratorSystematic(const Options &opts);
    ~PatternCollectionGeneratorSystematic() = default;

    virtual PatternCollectionInformation generate(
        std::function<bool(const Pattern &)> handle_pattern = nullptr) override;

    static void add_options_to_parser(OptionParser &parser);
};
}

#endif
