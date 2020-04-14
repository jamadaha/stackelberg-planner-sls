#! /usr/bin/env python

from lab.parser import Parser
from collections import defaultdict
import re

eval = Parser()



regexps = [re.compile("Optimally solved follower subproblems: (?P<optimally_solved_subproblems>(\d+))"),
           re.compile("Resulting Pareto-frontier: (?P<pareto_frontier>(.*))s"),
           re.compile("Follower search time: (?P<follower_time>(.*))s"), 
           re.compile("Leader search time: (?P<leader_time>(.*))"), 
]

# Search time: 29.3736s
# Total time: 29.9618s
 
type_atr = {'follower_time' : lambda x : max(0.01, float(x)),
            'leader_time' : lambda x : max(0.01, float(x)),
            'pareto_frontier' : str,
            "optimally_solved_subproblems" : int
        }

def parse_regexps (content, props):
    for l in content.split("\n"):
        for reg in regexps:
            mx = reg.match(l)
            if mx:
                data = mx.groupdict()
                for item in data:
                    props[item] = type_atr[item](data[item])
                break
            
eval.add_function(parse_regexps) 


