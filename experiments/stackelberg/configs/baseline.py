import configs

import sys

from collections import defaultdict

REVISION = "a4dd9da4d0af1b1134dcf2213b963e15415fc848"
SERVERS = "old_servers" 

CONFIGS = defaultdict(list)

CONFIGS["baseline"].append(configs.Config('baseline-lmcut', 'baseline-lmcut', ["--heuristic", "h1=deadpdbs(max_time=120)", "--heuristic", "h2=lmcut", "--search", "fixsearch(search_engine=astar(max([h1,h2]), pruning=null), attack_heuristic=h2, initial_attack_budget=2147483647, initial_fix_budget=2147483647, attack_op_dom_pruning=false, sort_fix_ops=true, greedy=false, upper_bound_pruning=true)"], REVISION, SERVERS))
