import configs

import sys

from collections import defaultdict

REVISION = "9e46b2ee32008462627cf810141dd06e6786856e"
SERVERS = "all_servers" 

CONFIGS = defaultdict(list)

CONFIGS["symbolic_leader"] += [configs.Config('ss-sbd', 'ss-sbd', ["--search", "sym_stackelberg(optimal_engine=symbolic())"], REVISION, SERVERS),
                               configs.Config('ss-sbd-origorder', 'ss-sbd-origorder', ["--search", "sym_stackelberg(optimal_engine=symbolic(stackelberg_variable_order=false))"], REVISION, SERVERS)
]





