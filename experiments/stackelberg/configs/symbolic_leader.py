import configs

import sys

from collections import defaultdict

REVISION = "9a79cb7c1a6ebce32404de8749f67e2e32c17509"
SERVERS = "all_servers" 

CONFIGS = defaultdict(list)

CONFIGS["symbolic_leader"] += [configs.Config('ss-sbd', 'ss-sbd', ["--search", "sym_stackelberg(optimal_engine=symbolic())"], REVISION, SERVERS),
                               configs.Config('ss-sbd-origorder', 'ss-sbd-origorder', ["--search", "sym_stackelberg(optimal_engine=symbolic, stackelberg_variable_order=false)"], REVISION, SERVERS)
]





