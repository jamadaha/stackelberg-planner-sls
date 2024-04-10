import re
import sys
import random
import argparse
import os
from shutil import copyfile
import json
from pprint import pprint
import matplotlib.pyplot as plt
import numpy as np
import ntpath
from matplotlib.ticker import FuncFormatter
from matplotlib.ticker import ScalarFormatter
import matplotlib.cm as cm
import math
from scipy import integrate

MAX_INT = 2147483647
EPSILON = 2.220446049250313e-16

def parse_properties_file(problem_file_name):
    problem_file = open(problem_file_name, "r")
    content = json.load(problem_file)
    problem_file.close()
    #pprint(content)
    domain_and_num_con = content['domain']
    i = domain_and_num_con.find("-tc")
    domain = domain_and_num_con[0:i]
    # HACK!!!
    if domain == "pentesting-large-robustness-rs42":
        domain = "pentesting-robustness-rs42"
    num_con = domain_and_num_con[i+3:]
    problem = content['problem']
    coverage = content['coverage']
    config = content['config_nick']
    #print domain_and_num_con
    #print domain
    #print num_con
    #print problem
    #print coverage
    #print config
    return domain, num_con, config, problem, coverage


p = argparse.ArgumentParser(description="")
p.add_argument("--dir", nargs='+', type=str, help="The directories which should be crawled", default=None)
p.add_argument("--configs", nargs='+', type=str, help="The interesting config which you want to compare", default=None)
p.add_argument("--domain", type=str, help="The interesting domain", default=None)
p.add_argument("--tcs", nargs='+', type=str, help="The interesting total number of connections", default=None)
p.add_argument("--name-suffix", type=str, help="The output filename suffix", default="")
args = p.parse_args(sys.argv[1:])

interesting_configs = args.configs
interesting_domain = args.domain
interesting_num_cons = args.tcs

#pareto_paths = []
dic = {}

for dir in args.dir:
    files_in_dir = os.listdir(dir)
    #print files_in_dir

    all_subdirs = [x[0] for x in os.walk(dir)]
    for subdir in all_subdirs:
        #print subdir
        if re.match(".*/\d{5}", subdir) is not None:
            files_in_subdir = os.listdir(subdir)
            for file in files_in_subdir:
                if file.find("properties") != -1:
                    domain, num_con, config, problem, coverage = parse_properties_file(os.path.join(subdir, file))
                    if domain == interesting_domain and (interesting_num_cons is None or num_con in interesting_num_cons) and config in interesting_configs and coverage == 1:
                        if os.path.exists(os.path.join(subdir, "pareto_frontier.json")) != -1:
                            if problem not in dic:
                                dic[problem] = dict()
                            if num_con not in dic[problem]:
                                dic[problem][num_con] = dict()
                            dic[problem][num_con][config] = os.path.join(subdir, "pareto_frontier.json")
                            #pareto_paths.append(os.path.join(subdir, "pareto_frontier.json"))
                        else:
                            print "No pareto_frontier.json found eventhough coverage was 1, for: " + domain_and_num_con + " " + config + " " + problem
                            exit(1)
                    break


rel_diff_sum = 0.0
rel_diff_num = 0
for problem, problem_dic in dic.iteritems():
    for num_con, num_con_dic in problem_dic.iteritems():
        functions = []
        max_x = 0
        x_arrays = []
        y_arrays = []
        i = 0
        for config, pareto_path in num_con_dic.iteritems():
            x_array = []
            y_array = []
            print pareto_path
            with open(pareto_path) as pareto_file:
                pareto = json.load(pareto_file)
                for entry in pareto[0:]:
                    x = entry['defender cost']
                    y = abs(entry['attacker cost'])
                    if y == MAX_INT:
                        s = entry['sequences']
                        #print s
                    max_x = max(max_x, x)
                    x_array.append(x)
                    y_array.append(y)

                if y_array[0] == MAX_INT:
                    #print pareto_path + "skipped, because not solvable in initial state"
                    continue
                x_array.append(MAX_INT)
                y_array.append(y_array[len(y_array)-1])
                x_arrays.append(x_array)
                y_arrays.append(y_array)
                f = lambda x, i: y_arrays[i][next(y[0] for y in enumerate(x_arrays[i]) if y[1] > x) - 1]
                functions.append(f)
                i += 1

        if len(functions) > 1:
            print x_arrays
            print y_arrays
            area1 = integrate.quad(functions[0], 0, max_x, args=(0))
            area2 = integrate.quad(functions[1], 0, max_x, args=(1))
            print area1
            print area2
            diff = abs(area2[0] - area1[0])
            low = min(area1[0], area2[0])
            high = max(area1[0], area2[0])
            rel_diff = max(1.0, high / max(low, 1.0))
            print diff
            print rel_diff
            rel_diff_sum += rel_diff
            rel_diff_num += 1

print "average relative difference = " + str(rel_diff_sum/rel_diff_num)