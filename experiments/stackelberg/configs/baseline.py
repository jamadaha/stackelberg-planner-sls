import configs

import sys

from collections import defaultdict

REVISION = "dfe742a2a1d7c56565a01f61ddece35ac27b244d"
SERVERS = "all_servers" 

CONFIGS = defaultdict(list)

CONFIGS["baseline"] += [configs.Config('original-lmcut', 'original-lmcut', ["--heuristic", "h1=deadpdbs(max_time=120)", "--heuristic", "h2=lmcut", "--search", "fixsearch(search_engine=astar(max([h1,h2]), pruning=null), attack_heuristic=h2, initial_attack_budget=2147483647, initial_fix_budget=2147483647, attack_op_dom_pruning=false, sort_fix_ops=true, greedy=false, upper_bound_pruning=true)"], REVISION, SERVERS),
                        configs.Config('baseline-lmcut', 'baseline-lmcut', ["--heuristic", "h1=deadpdbs(max_time=120)", "--heuristic", "h2=lmcut", "--search", "stackelberg(search_engine=astar(max([h1,h2])), follower_heuristic=h2)"], REVISION, SERVERS),
                        configs.Config('baseline-sbd', 'baseline-sbd', ["--search", "stackelberg(search_engine=sbd())"], REVISION, SERVERS)

]


