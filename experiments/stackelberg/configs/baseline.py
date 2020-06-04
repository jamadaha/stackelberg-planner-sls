import configs

import sys

from collections import defaultdict

REVISION = "8afa96ef3ce86a6b39ea031ac396f7b732dc43a5"
SERVERS = "new_servers" 

CONFIGS = defaultdict(list)

CONFIGS["baseline"] += [
    configs.Config('original-lmcut-pdbs', 'original-lmcut-pdbs', ["--heuristic", "h1=deadpdbs(max_time=120)", "--heuristic", "h2=lmcut", "--search", "fixsearch(search_engine=astar(max([h1,h2]), pruning=null), attack_heuristic=h2, initial_attack_budget=2147483647, initial_fix_budget=2147483647, attack_op_dom_pruning=false, sort_fix_ops=true, greedy=false, upper_bound_pruning=true)"], REVISION, SERVERS),
    configs.Config('baseline-lmcut-pdbs', 'baseline-lmcut-pdbs', ["--heuristic", "h1=deadpdbs(max_time=120)", "--heuristic", "h2=lmcut", "--search", "stackelberg(search_engine=astar(max([h1,h2])), follower_heuristic=h2)"], REVISION, SERVERS),
    configs.Config('baseline-lmcut', 'baseline-lmcut', ["--heuristic", "h2=lmcut", "--search", "stackelberg(search_engine=astar(h2), follower_heuristic=h2)"], REVISION, SERVERS),
    configs.Config('baseline-sbd', 'baseline-sbd', ["--search", "stackelberg(search_engine=sbd())"], REVISION, SERVERS)

]



