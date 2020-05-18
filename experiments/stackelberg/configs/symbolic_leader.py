import configs

import sys

from collections import defaultdict

REVISION = "d7c615d35bca3add6841ff36c7587ff8b00ced07"
SERVERS = "all_servers" 

CONFIGS = defaultdict(list)

CONFIGS["symbolic_leader"] += [configs.Config('ss-sbd', 'ss-sbd', ["--search", "sym_stackelberg(optimal_engine=symbolic())"], REVISION, SERVERS),
                               configs.Config('ss-sbd-origorder', 'ss-sbd-origorder', ["--search", "sym_stackelberg(optimal_engine=symbolic, stackelberg_variable_order=false)"], REVISION, SERVERS)
]





