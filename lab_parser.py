#! /usr/bin/env python

from lab.parser import Parser
from collections import defaultdict
from ast import literal_eval

import re

eval = Parser()




regexps = [re.compile("Optimally solved follower subproblems: (?P<optimally_solved_subproblems>(\d+))"),
           re.compile("Pareto-frontier: (?P<pareto_frontier>(.*))"),
           re.compile("Pareto-frontier size: (?P<pareto_frontier_size>(\d+))"),
           re.compile("Follower search time: (?P<follower_time>(.*))s"), 
           re.compile("Leader search time: (?P<leader_time>(.*))s"), 
]

def parse_pareto_frontier(x):
    x = x.strip()
    if " " in x:
        return map(literal_eval, x.split(" "))
    else:
        return [literal_eval(x)]

type_atr = {'follower_time' : lambda x : max(0.01, float(x)),
            'leader_time' : lambda x : max(0.01, float(x)),
            'pareto_frontier' : parse_pareto_frontier,
            'pareto_frontier_size' : int,
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

def set_coverage (content, props):
    if 'pareto_front' in props:
        props['coverage'] = 1
        
        
eval.add_function(set_coverage) 


eval.parse()
