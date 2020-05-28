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
           re.compile("Optimal solver time: (?P<optimal_solver_time>(.*))s"), 
           re.compile("Cost-bounded solver time: (?P<cost_bounded_solver_time>(.*))s"), 
           re.compile("Leader search time: (?P<leader_time>(.*))s"),
           re.compile("Total follower searches: (?P<total_follower_searches>(\d+))"),     
           re.compile("Solved by optimal solver: (?P<optimal_solver_searches>(\d+))"),
           re.compile("Solved by cost bounded solver: (?P<cost_bounded_solver_searches>(\d+))"),   
]

def parse_pareto_frontier(x):
    x = x.strip()
    if " " in x:
        return map(literal_eval, x.split(" "))
    else:
        return [literal_eval(x)]

type_atr = {'follower_time' : lambda x : max(0.01, float(x)),
            'leader_time' : lambda x : max(0.01, float(x)),
            'optimal_solver_time' : lambda x : max(0.01, float(x)),
            'cost_bounded_solver_time' : lambda x : max(0.01, float(x)),
            'pareto_frontier' : parse_pareto_frontier,
            'pareto_frontier_size' : int,
            "optimally_solved_subproblems" : int,
            'total_follower_searches' : int,
            'optimal_solver_searches' : int,
            'cost_bounded_solver_searches' : int
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
            



def change_domain(content, props):

    if "-rs42-" in props["domain"]:
        new_domain_name, problem_appendix = props["domain"].split("-rs42-")    
        props["domain"] = new_domain_name

        assert problem_appendix.startswith("tc") and not "tc" in props["problem"]
        props["problem"] = "{}-{}".format(props["problem"], problem_appendix)

     

def set_coverage (content, props):
    if 'pareto_frontier' in props:
        props['coverage'] = 1

eval.add_function(parse_regexps)        
eval.add_function(set_coverage)
eval.add_function(change_domain) 


eval.parse()
