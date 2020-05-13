import configs

import sys

from collections import defaultdict

REVISION = "f999d1b1ec7a1c7b5e4494054857859fb55e41ed"
SERVERS = "all_servers" 

CONFIGS = defaultdict(list)

CONFIGS["symbolic_leader"] += [configs.Config('ss-sbd', 'ss-sbd', ["--search", "sym_stackelberg(optimal_engine=symbolic())"], REVISION, SERVERS)
]





