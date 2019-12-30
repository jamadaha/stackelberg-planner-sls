import configs
from configs_simulation import *

import sys

from collections import defaultdict

REVISION = "a40e8a36318c644f619abf7a9bf6a004ceae7615"
SERVERS = "old_servers" 

CONFIGS = defaultdict(list)

CONFIGS["baseline"].append(configs.Config('baseline-lmcut', 'baseline-lmcut', ["--preprocess-options", "-asd", "--search-options", "--heuristic", "h1=deadpdbs(max_time=120)", "--heuristic", "h2=lmcut", "--search", "fixsearch(search_engine=astar(max([h1,h2]), pruning=null), attack_heuristic=h2, initial_attack_budget=2147483647, initial_fix_budget=2147483647, attack_op_dom_pruning=false, sort_fix_ops=true, greedy=false, upper_bound_pruning=true)"], REVISION, SERVERS))
