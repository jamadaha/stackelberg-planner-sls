import configs

import sys

from collections import defaultdict

REVISION = "8afa96ef3ce86a6b39ea031ac396f7b732dc43a5"
SERVERS = "new_servers" 

CONFIGS = defaultdict(list)

CONFIGS["symbolic_leader"] += [configs.Config('ss-sbd', 'ss-sbd', \
                ["--search", "sym_stackelberg(optimal_engine=symbolic(plan_reuse_minimal_task_upper_bound=false, plan_reuse_upper_bound=false), upper_bound_pruning=false)"], REVISION, SERVERS),
 configs.Config('ss-sbd-up', 'ss-sbd-up', ["--search", "sym_stackelberg(optimal_engine=symbolic(plan_reuse_minimal_task_upper_bound=false, plan_reuse_upper_bound=false), upper_bound_pruning=true)"], REVISION, SERVERS), 
 configs.Config('ss-sbd-ubreuse', 'ss-sbd-ubreuse', ["--search", "sym_stackelberg(optimal_engine=symbolic(plan_reuse_minimal_task_upper_bound=false, plan_reuse_upper_bound=true), upper_bound_pruning=false)"], REVISION, SERVERS), 
 # configs.Config('ss-sbd-up-ubreuse', 'ss-sbd-up-ubreuse', ["--search", "sym_stackelberg(optimal_engine=symbolic(plan_reuse_minimal_task_upper_bound=true, plan_reuse_upper_bound=true), upper_bound_pruning=true)"], REVISION, SERVERS),
 configs.Config('ss-sbd-up-ubreuse-tlim', 'ss-sbd-up-ubreuse-tlim', ["--search", "sym_stackelberg(optimal_engine=symbolic(plan_reuse_minimal_task_upper_bound=true, plan_reuse_upper_bound=true, force_bw_search_minimum_task_seconds=30, time_limit_seconds_minimum_task=300), upper_bound_pruning=true)"], REVISION, SERVERS),
]






configs.Config('ss-lmcut', 'ss-lmcut', ["--search", "sym_stackelberg(optimal_engine=explicit(search_engine=astar(lmcut()), is_optimal_solver=true, plan_reuse_upper_bound=false))"], REVISION, SERVERS),                              
