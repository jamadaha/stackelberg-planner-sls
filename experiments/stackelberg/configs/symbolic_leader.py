import configs

import sys

from collections import defaultdict

REVISION = "9d28b90479151ddc85998ca5d9e5fd27f4269bd0"
SERVERS = "all_servers" 

CONFIGS = defaultdict(list)

CONFIGS["symbolic_leader"] += [configs.Config('ss-sbd', 'ss-sbd', ["--search", "sym_stackelberg(optimal_engine=symbolic(plan_reuse_minimal_task_upper_bound=false, plan_reuse_upper_bound=false), upper_bound_pruning=false)"], "e6c2e046975f2de076388dddbfd3701a8e595f01", SERVERS),
                               configs.Config('ss-lmcut', 'ss-lmcut', ["--search", "sym_stackelberg(optimal_engine=explicit(search_engine=astar(lmcut())))"], "1a149cd24770af8587ce49d968ff298c721c9eb7", SERVERS),
                               #configs.Config('ss-sbd-origorder', 'ss-sbd-origorder', ["--search", "sym_stackelberg(optimal_engine=symbolic, stackelberg_variable_order=false)"], REVISION, SERVERS)
                               configs.Config('ss-sbd-up', 'ss-sbd-up', ["--search", "sym_stackelberg(optimal_engine=symbolic(plan_reuse_minimal_task_upper_bound=false, plan_reuse_upper_bound=false), upper_bound_pruning=true)"], REVISION, SERVERS),
                               configs.Config('ss-sbd-ubreuse', 'ss-sbd-up-ubreuse', ["--search", "sym_stackelberg(optimal_engine=symbolic(plan_reuse_minimal_task_upper_bound=false, plan_reuse_upper_bound=true), upper_bound_pruning=false)"], REVISION, SERVERS),
                               configs.Config('ss-sbd-up-ubreuse', 'ss-sbd-up-ubreuse', ["--search", "sym_stackelberg(optimal_engine=symbolic(plan_reuse_minimal_task_upper_bound=true, plan_reuse_upper_bound=true), upper_bound_pruning=true)"], REVISION, SERVERS),
]





