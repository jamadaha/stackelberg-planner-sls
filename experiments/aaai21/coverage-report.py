#!/usr/bin/python3

import json
import  sys
from collections import defaultdict
import statistics 

import argparse
parser = argparse.ArgumentParser()
parser.add_argument("properties_file")
parser.add_argument("--tc", action="store_true", help="include tc")

parser.add_argument("--goals", action="store_true", help="include tc")
parser.add_argument("--onlytcall", action="store_true", help="include only tcall")

parser.add_argument("--category", help="only for a category")

args = parser.parse_args()
    
def pretty_print_domain(dom):
    dom = dom.replace("aaai18-no-mystery", "mystery")
    dom = dom.replace("aaai18", "")
    dom = dom.replace("aaai21", "")
    dom = dom.replace("soft", "")
    dom = dom.replace("logistics98", "logistics")
    dom = dom.replace("strips", "")
    dom = dom.replace("opt11", "")
    dom = dom.replace("opt14", "")    
    dom = dom.replace("total", "$\\sum$")
    dom = dom.replace("notankage", "")    

    dom = "-".join([x for x in dom.split("-") if x])
    
    return dom.capitalize()

def merge_equivalent(dom):
    dom = dom.replace("driving", "")
    dom = dom.replace("fixed", "")
    dom = dom.replace("walls", "")
        
    dom = "-".join([x for x in dom.split("-") if x])
    
    return dom


def highlight (coverage, highlighting, algo, dom):
    category_highlight = "lmcut" if "lmcut" in algo else "sbd"
    if highlighting[(category_highlight, dom)]  == coverage[(algo, dom)]:
        return f"\\bf {coverage[(algo, dom)]}"
    else:
        return str(coverage[(algo, dom)])

pareto_front_len = defaultdict()
highlighting = defaultdict(int)

domains_per_category = defaultdict(set)
algorithm_list = ['baseline-lmcut', 'ss-lmcut', 'ss-lmcut-ubreuse', 'ss-up-lmcut-ubreuse', 'ss-lmcut-up-ubreuse-cbfflb-1s', 'baseline-sbd', 'ss-sbd', 'ss-sbd-ubreuse', 'ss-sbd-up-ubreuse-tlim', 'ss-sbd-up-ubreuse-cbfflb-1s']


all_algorithms = set()
coverage = defaultdict(int)
num_instances = defaultdict(int)

with open(args.properties_file) as json_file:
    properties = json.load(json_file)
    for a, data in properties.items():
        if "coverage" not in data:
            continue
        all_algorithms.add(data["config"])
        
        if data["config"] not in algorithm_list:
            continue
        domain, config = data["domain"], data["config"]

        if "fuel" in domain:
            continue
        
        category = "aaai18"
        if "aaai21" in domain:
            category = "aaai21-soft" if "-soft" in domain else "aaai21"

        domain = merge_equivalent(domain)
        if args.goals and "translator_goal_facts" in data:
            domain += "-{:02d}".format(data["translator_goal_facts"])
        if args.tc or args.onlytcall:
            if data["problem"].split("-")[-1] == "tcall":
                domain += "-all"
            elif not args.onlytcall:
                domain += "-{:03d}".format(int(data["problem"].split("-")[-1].replace("tc", "")))
            else:
                domain += "-tcX"                
        domains_per_category[category].add(domain)
        if data["coverage"] == 1:            
            coverage[(config, domain)] += 1
            coverage[(config, f"total-{category}")] += 1
            
        num_instances[(config, domain)] += 1
        num_instances[(config, f"total-{category}")] += 1

        if not (category, domain) in pareto_front_len:
            pareto_front_len[(category, domain)] = defaultdict(float)
        if "pareto_frontier" in data and "baseline" not in config:
            pareto_front_len[(category, domain)] [domain + data["problem"]] = float(len(data["pareto_frontier"]))
            
        if "pareto_frontier_size" in data and "baseline" not in config:
            pareto_front_len[(category, domain)] [domain + data["problem"]] = float(data["pareto_frontier_size"])

        category_highlight = "lmcut" if "lmcut" in config else "sbd"
        highlighting[(category_highlight, domain)] = max(highlighting[(category_highlight, domain)], coverage[(config, domain)])
        highlighting[(category_highlight, f"total-{category}")] = max(highlighting[(category_highlight, f"total-{category}")], coverage[(config, f"total-{category}")])


num_instances_domain = defaultdict(int)
for (alg, dom) in num_instances:
    assert not dom in num_instances_domain or num_instances_domain[dom] == num_instances[(alg, dom)] or "total" in dom, f"Error {alg} has {num_instances[(alg, dom)]} instances in {dom} instead of {num_instances_domain[dom]}"  
    num_instances_domain[dom]   = max(num_instances_domain[dom], num_instances[(alg, dom)])


for category in ["aaai18", "aaai21", "aaai21-soft"]:
    for dom in domains_per_category[category]:
        values = pareto_front_len[(category, dom)].values()

        if values: 
            pareto_front_len[(category, dom, "max")] = str(int(max(values)))
            pareto_front_len[(category, dom, "avg")] = "{:.2f}".format(statistics.mean(values))
        else:
            #print (f"No values for {category} {dom}")
            pareto_front_len[(category, dom, "max")] = "---"
            pareto_front_len[(category, dom, "avg")] = "---"

    pareto_front_len[(category, f"total-{category}" , "max")] = ""
    pareto_front_len[(category, f"total-{category}" , "avg")] = ""

print ("% {} \\\\".format( " & ".join(["domain"] + algorithm_list)))

rows = []
for category in ["aaai18", "aaai21", "aaai21-soft"] if not args.category else [args.category]:
    if category == "aaai18":
        rows.append ("\\multirow{7}{*}{\\benchmarkprev}")
    elif category == "aaai21":
        rows.append ("\\multirow{7}{*}{\\benchmarknew}")
    else:
        rows.append ("\\multirow{7}{*}{\\benchmarksoft}")
        
    for dom in sorted(domains_per_category[category]) + [f"total-{category}"]:
        rows.append (" & ".join(["" if "total-" not in dom else "\\midrule", pretty_print_domain(dom), f"({num_instances_domain[dom]})", pareto_front_len[(category, dom, "avg")], pareto_front_len[(category, dom, "max")]  ] + [highlight(coverage, highlighting, alg, dom) for alg in algorithm_list]) + "\\\\\n")

    rows.append ("\\midrule")

print("\n".join(rows[:-1]))
print ("\\bottomrule")
             

