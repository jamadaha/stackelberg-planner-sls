import configs

import sys

from collections import defaultdict

REVISION = "7176b68a37a870e7446e35bbcc228990205997a6"
SERVERS = "new_servers" 

CONFIGS = defaultdict(list)

CONFIGS["symbolic_leader_lmcut"] += [
    configs.Config('ss-lmcut', 'ss-lmcut', ["--search", "sym_stackelberg(optimal_engine=explicit(search_engine=astar(lmcut()), is_optimal_solver=true, plan_reuse_upper_bound=false), upper_bound_pruning=false)"], REVISION, SERVERS),
    configs.Config('ss-lmcut-pdbs', 'ss-lmcut-pdbs', ["--search", "sym_stackelberg(optimal_engine=explicit(search_engine=astar(max([deadpdbs(max_time=120),lmcut])), is_optimal_solver=true, plan_reuse_upper_bound=false), upper_bound_pruning=false)"], REVISION, SERVERS),
    configs.Config('ss-lmcut-ubreuse', 'ss-lmcut-ubreuse', ["--search", "sym_stackelberg(optimal_engine=explicit(search_engine=astar(lmcut()), is_optimal_solver=true, plan_reuse_upper_bound=true), upper_bound_pruning=false)"], REVISION, SERVERS),
    configs.Config('ss-lmcut-pdbs-ubreuse', 'ss-lmcut-pdbs-ubresuse', ["--search", "sym_stackelberg(optimal_engine=explicit(search_engine=astar(max([deadpdbs(max_time=120),lmcut])), is_optimal_solver=true, plan_reuse_upper_bound=true), upper_bound_pruning=false)"], REVISION, SERVERS),
]





