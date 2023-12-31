#include "validation.h"

#include "../utilities.h"
#include "../globals.h"

#include <algorithm>
#include <iostream>

using namespace std;

namespace pdbs
{
void validate_and_normalize_pattern(Pattern &pattern)
{
    /*
      - Sort by variable number and remove duplicate variables.
      - Warn if duplicate variables exist.
      - Error if patterns contain out-of-range variable numbers.
    */
    sort(pattern.begin(), pattern.end());
    auto it = unique(pattern.begin(), pattern.end());
    if (it != pattern.end()) {
        pattern.erase(it, pattern.end());
        cout << "Warning: duplicate variables in pattern have been removed"
             << endl;
    }
    if (!pattern.empty()) {
        if (pattern.front() < 0) {
            cerr << "Variable number too low in pattern" << endl;
            exit_with(EXIT_CRITICAL_ERROR);
        }
        int num_variables = g_variable_domain.size();
        if (pattern.back() >= num_variables) {
            cerr << "Variable number too high in pattern" << endl;
            exit_with(EXIT_CRITICAL_ERROR);
        }
    }
}

void validate_and_normalize_patterns(PatternCollection &patterns)
{
    /*
      - Validate and normalize each pattern (see there).
      - Warn if duplicate patterns exist.
    */
    for (Pattern &pattern : patterns) {
        validate_and_normalize_pattern(pattern);
    }
    PatternCollection sorted_patterns(patterns);
    sort(sorted_patterns.begin(), sorted_patterns.end());
    auto it = unique(sorted_patterns.begin(), sorted_patterns.end());
    if (it != sorted_patterns.end()) {
        cout << "Warning: duplicate patterns have been detected" << endl;
    }
}
}
