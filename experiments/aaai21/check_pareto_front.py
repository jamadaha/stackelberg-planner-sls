#!/usr/bin/python3

import json
import  sys
from collections import defaultdict

if len(sys.argv) < 2:
    print ("Usage: ./check_pareto_front.py <properties_file>")
    exit(0)
    

pareto_fronts = defaultdict(set)
pareto_fronts_list = defaultdict(list)

with open(sys.argv[1]) as json_file:
    properties = json.load(json_file)
    for a, data in properties.items():
        if "pareto_frontier" in data:
            pareto_fronts[(data["domain"], data["problem"])].add(tuple(map(tuple, data["pareto_frontier"])))
            pareto_fronts_list[(data["domain"], data["problem"])].append((data["config_nick"][:12], tuple(map(tuple, data["pareto_frontier"]))))


    for instance, fronts in pareto_fronts.items():
        if len(fronts) > 1:

            (domain, problem) = instance
           
            if fronts == {((0, 0),), ((0, 2147483647),)}:
                continue

            print (f"{domain} {problem}")
            for a, b in pareto_fronts_list[instance]:
                print(f"    {a}: {b}")
                

            print("")
            
