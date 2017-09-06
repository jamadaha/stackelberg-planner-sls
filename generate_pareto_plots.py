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

MAX_INT = 2147483647

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
    num_con = int(domain_and_num_con[i+3:])
    problem = content['problem']
    coverage = content['coverage']
    config = content['config_nick']
    #print domain_and_num_con
    #print domain
    #print num_con
    #print problem
    #print coverage
    #print config
    return domain_and_num_con, config, problem, coverage


p = argparse.ArgumentParser(description="")
p.add_argument("--dir", nargs='+', type=str, help="The directories which should be crawled", default=None)
p.add_argument("--config", type=str, help="The interesting config which you want to plot", default=None)
p.add_argument("--domain", type=str, help="The interesting domain which you want to plot", default=None)
p.add_argument("--name-suffix", type=str, help="The output filename suffix", default="")
args = p.parse_args(sys.argv[1:])

interesting_config = args.config
interesting_domain = args.domain

pareto_paths = []

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
                    domain_and_num_con, config, problem, coverage = parse_properties_file(os.path.join(subdir, file))
                    if domain_and_num_con == interesting_domain and config == interesting_config and coverage == 1:
                        if os.path.exists(os.path.join(subdir, "pareto_frontier.json")) != -1:
                            pareto_paths.append(os.path.join(subdir, "pareto_frontier.json"))
                        else:
                            print "No pareto_frontier.json found eventhough coverage was 1, for: " + domain_and_num_con + " " + config + " " + problem
                            exit(1)
                    break

FONTSIZE = 18
legend_FONTSIZE = 22
LINEWIDTH = 1
MARKERWIDTH = 30
MEW = 2

#colors = ['b', 'r', 'g', 'k', 'y', 'm', 'c']
markers = ['x', 's', '+', '.', 'd']
filled_markers = ['o', 'v', '^', '<', '>', '8', 's', 'p', '*', 'h', 'H', 'D', 'd', 'P', 'X']
fillstyles = ['full', 'none', 'full', 'none', 'none']
MEWs = ['1', '1', '1', '1', '1']

fig, ax = plt.subplots(figsize=(15,7))

output_file = "scatter_" + interesting_domain + "_" + interesting_config + ".pdf"

x_array =[]
y_array = []

plt.style.use('grayscale')

i = 0
max_y = 0
x_arrays = []
y_arrays = []
for pareto_path in pareto_paths:
            x_array = []
            y_array = []
            with open(pareto_path) as pareto_file:
                pareto = json.load(pareto_file)
                #x_array.append(pareto[1]['cost']/10)
                #y_array.append(pareto[0]['reward'])
                last_x = 1.0
                last_y = 1.0
                for entry in pareto[0:]:
                    x = entry['defender cost']
                    y = abs(entry['attacker cost'])
                    if y != MAX_INT:
                        max_y = max(max_y, y)
                    if x == 0:
                        x_array.append(x)
                        y_array.append(y)
                        continue
                    d = 1.0 # math.sqrt(pow(math.log10(x) - math.log10(last_x), 2) + pow((y*5) - (last_y*5), 2))
                    #if x < 10 or float(x)/last_x > 1.2 or y == 0 or last_y/float(y) > 1.2:
                    if x < 10 or d > 0.17:
                        x_array.append(x)
                        y_array.append(y)
                        last_x = max(x, 1.0)
                        last_y = y
                if y_array[0] == MAX_INT:
                    print pareto_path + "skipped, because not solvable in initial state"
                    continue
                x_arrays.append(x_array)
                y_arrays.append(y_array)
                i += 1

inf_representation = 1.5 * max_y
y_axis_max = 1.6 * max_y
for y_array in y_arrays:
    for i in range(len(y_array)):
        if y_array[i] == 2147483647:
            y_array[i] = inf_representation

colors = iter(cm.rainbow(np.linspace(0, 1, len(x_arrays))))
for i in range(len(x_arrays)):
    x_array = x_arrays[i]
    y_array = y_arrays[i]
# write: '--' + markers[i] for line between markers
    line = ax.plot(x_array, y_array, '--' + filled_markers[i], label='a', c=next(colors))
#line = plt.plot(x_array, y_array, markers[0], fillstyle=fillstyles[0], markersize=MARKERWIDTH, \
#                            linewidth=LINEWIDTH, \
#                            markeredgewidth=MEWs[0], \
#                            label=interesting_domain)
#plt.setp(line, color='black')


#for i in range(0, len(x_arrays)):
#    plt.plot(x_arrays[i], y_arrays[i], '--' + markers[i], fillstyle=fillstyles[i], markersize=MARKERWIDTH, \
#             linewidth=LINEWIDTH, \
#             label=attacker + ' vs. ' + dc + ', scenario ' + s)

ax.legend(loc='best', fontsize=legend_FONTSIZE)

plt.xlabel('Leader Cost',fontsize=FONTSIZE)
plt.ylabel('Follower Cost',fontsize=FONTSIZE)
plt.xticks(fontsize = FONTSIZE)
plt.yticks(fontsize = FONTSIZE)
#plt.xscale('symlog')
ax.set_ylim(ymin=0, ymax=y_axis_max)

y_ticks = ax.yaxis.get_majorticklocs()
print y_ticks
y_ticks = y_ticks[:-1]
y_ticks[-1] = inf_representation
ax.yaxis.set_ticks(y_ticks)
print y_ticks


ax.grid(color='lightgrey', linestyle='--', linewidth=0.5)

ax.yaxis.set_major_formatter(FuncFormatter(lambda x, pos: u"\u221E" if x==(inf_representation) else x))
#default_formatter = ax.get_xaxis().get_major_formatter()
#ax.xaxis.set_major_formatter(FuncFormatter(lambda x, pos: 0 if pos == 2 else default_formatter(x,pos)))

plt.savefig(output_file, format='pdf', bbox_inches='tight')
#plt.show()

