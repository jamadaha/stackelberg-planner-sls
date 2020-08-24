#!/usr/bin/python3

import json
import  sys
from collections import defaultdict

if len(sys.argv) < 2:
    print ("Usage: ./coverage-report.py <properties_file>")
    exit(0)


    
def pretty_print_domain(dom):
    dom = dom.replace("aaai18-no-mystery", "mystery")
    dom = dom.replace("aaai18", "")
    dom = dom.replace("aaai21", "")
    dom = dom.replace("soft", "")
    dom = dom.replace("logistics98", "logistics")
    dom = dom.replace("strips", "")
    dom = dom.replace("opt11", "")
    dom = dom.replace("opt14", "")    
    dom = dom.replace("total", "\\midrule $\\sum$")
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

highlighting = defaultdict(int)

domains_per_category = defaultdict(set)
algorithm_list = ['baseline-lmcut', 'ss-lmcut', 'ss-lmcut-ubreuse', 'baseline-sbd', 'ss-sbd', 'ss-sbd-ubreuse', 'ss-sbd-up', 'ss-sbd-up-ubreuse-tlim', 'ss-sbd-ubreuse-cbff-1s']

all_algorithms = set()
coverage = defaultdict(int)
num_instances = defaultdict(int)

with open(sys.argv[1]) as json_file:
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
        domains_per_category[category].add(domain)
        if data["coverage"] == 1:            
            coverage[(config, domain)] += 1
            coverage[(config, f"total-{category}")] += 1
            
        num_instances[(config, domain)] += 1
        num_instances[(config, f"total-{category}")] += 1

        category_highlight = "lmcut" if "lmcut" in config else "sbd"
        highlighting[(category_highlight, domain)] = max(highlighting[(category_highlight, domain)], coverage[(config, domain)])
        highlighting[(category_highlight, f"total-{category}")] = max(highlighting[(category_highlight, f"total-{category}")], coverage[(config, f"total-{category}")])

num_instances_domain = {}
for (alg, dom) in num_instances:
    assert not dom in num_instances_domain or num_instances_domain[dom] == num_instances[(alg, dom)], f"Error {alg} has {num_instances[(alg, dom)]} instances in {dom} instead of {num_instances_domain[dom]}"  
    num_instances_domain[dom]   = num_instances[(alg, dom)]




    
print ("% {} \\\\".format( " & ".join(["domain"] + algorithm_list)))
for category in ["aaai18", "aaai21", "aaai21-soft"]:
    for dom in sorted(domains_per_category[category]) + [f"total-{category}"]:
        print (" & ".join([pretty_print_domain(dom), f"({num_instances_domain[dom]})"] + [highlight(coverage, highlighting, alg, dom) for alg in algorithm_list]) + "\\\\")
    print ("\\midrule")
             

# print (all_algorithms)
