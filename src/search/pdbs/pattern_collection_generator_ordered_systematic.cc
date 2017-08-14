#include "pattern_collection_generator_ordered_systematic.h"

#include "pattern_collection_generator_systematic.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "../globals.h"

#include <algorithm>
#include <cassert>
#include <iostream>

using namespace std;

namespace pdbs
{
PatternCollectionGeneratorOrderedSystematic::PatternCollectionGeneratorOrderedSystematic(
    const Options &opts)
    : pattern_max_size(opts.get<int>("pattern_max_size"))
{
}

PatternCollectionInformation
PatternCollectionGeneratorOrderedSystematic::generate(
    function<bool(const Pattern &)> handle_pattern)
{
    shared_ptr<PatternCollection> patterns = make_shared<PatternCollection>();
    pattern_max_size = min(pattern_max_size,
                           g_variable_domain.size());
    bool done = false;
    for (size_t pattern_size = 1; pattern_size < pattern_max_size; ++pattern_size) {
        cout << "Generating patterns for size " << pattern_size << endl;
        Options opts;
        opts.set<int>("pattern_max_size", pattern_size);
        opts.set<bool>("only_interesting_patterns", true);
        PatternCollectionGeneratorSystematic generator(opts);
        generator.generate([&](const Pattern & pattern) {
            if (pattern.size() == pattern_size) {
                patterns->push_back(pattern);
                if (handle_pattern) {
                    done = handle_pattern(pattern);
                }
            }
            return done;
        });
        if (done) {
            break;
        }
    }
    return PatternCollectionInformation(patterns);
}

void PatternCollectionGeneratorOrderedSystematic::add_options_to_parser(
    OptionParser &parser)
{
    parser.add_option<int>(
        "pattern_max_size",
        "max number of variables per pattern",
        "-1");
}


static PatternCollectionGenerator *_parse(OptionParser &parser)
{
    Options opts = parser.parse();
    if (parser.dry_run()) {
        return nullptr;
    }

    return new PatternCollectionGeneratorOrderedSystematic(opts);
}

static Plugin<PatternCollectionGenerator> _plugin("ordered_systematic",
        _parse);
}

