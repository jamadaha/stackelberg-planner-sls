import configs

import sys

from collections import defaultdict

REVISION = "1a149cd24770af8587ce49d968ff298c721c9eb7"
SERVERS = "all_servers" 

CONFIGS = defaultdict(list)

CONFIGS["symbolic_leader"] += [configs.Config('ss-sbd', 'ss-sbd', ["--search", "sym_stackelberg(optimal_engine=symbolic())"], "e6c2e046975f2de076388dddbfd3701a8e595f01", SERVERS),
                               configs.Config('ss-lmcut', 'ss-lmcut', ["--search", "sym_stackelberg(optimal_engine=explicit(search_engine=astar(lmcut())))"], REVISION, SERVERS),
                               configs.Config('ss-sbd-origorder', 'ss-sbd-origorder', ["--search", "sym_stackelberg(optimal_engine=symbolic, stackelberg_variable_order=false)"], REVISION, SERVERS)
]





