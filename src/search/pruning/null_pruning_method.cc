#include "null_pruning_method.h"

#include "../option_parser.h"
#include "../plugin.h"

using namespace std;

namespace null_pruning_method {
void NullPruningMethod::initialize() {
    PruningMethod::initialize();
    cout << "pruning method: none" << endl;
}

PruningMethod* _parse(OptionParser &parser) {
    parser.document_synopsis(
        "No pruning",
        "This is a skeleton method that does not perform any pruning, i.e., "
        "all applicable operators are applied in all expanded states. ");

    if (parser.dry_run()) {
        return nullptr;
    }

    return new NullPruningMethod();
}

Plugin<PruningMethod> _plugin("null", _parse);
}
