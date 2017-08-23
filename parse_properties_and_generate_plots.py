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


FONTSIZE = 18
legend_FONTSIZE = 22
LINEWIDTH = 1
MARKERWIDTH = 30
MEW = 2

colors = ['b', 'r', 'g', 'k', 'y', 'm', 'c']
markers = ['x', 's', '+', '.', 'd']
fillstyles = ['full', 'none', 'full', 'none', 'none']
MEWs = ['1', '1', '1', '1', '1']


dic = {}

def parse_properties_file(problem_file_name):
    problem_file = open(problem_file_name, "r")
    content = json.load(problem_file)
    problem_file.close()
    #pprint(content)
    domain_and_num_con = content['domain']
    i = domain_and_num_con.find("-tc")
    domain = domain_and_num_con[0:i]
    num_con = int(domain_and_num_con[i+3:])
    problem = content['problem']
    coverage = content['coverage']
    config = content['config_nick']
    print domain_and_num_con
    print domain
    print num_con
    print problem
    print coverage
    print config
    if domain not in dic:
        dic[domain] = dict()
    if config not in dic[domain]:
        dic[domain][config] = dict()
    if num_con not in dic[domain][config]:
        dic[domain][config][num_con] = dict()
    if problem not in dic[domain][config][num_con]:
        dic[domain][config][num_con][problem] = 0
    dic[domain][config][num_con][problem] = coverage

def sort_data_pints (x_array, y_array):
    x_array = list(x_array)
    old_pos_by_x_value = {}
    for i in range(len(x_array)):
        old_pos_by_x_value[x_array[i]] = i
    x_array.sort()

    new_pos_by_old_pos = {}
    for i in range(len(x_array)):
        new_pos_by_old_pos[old_pos_by_x_value[x_array[i]]] = i

    new_y_array = [0] * len(y_array)
    for i in range(len(y_array)):
        new_y_array[new_pos_by_old_pos[i]] = y_array[i]

    return x_array, new_y_array


def plot_coverage_for_domain(domain):
    fig, ax = plt.subplots(figsize=(15, 7))
    # plt.style.use('grayscale')

    i = 0
    x_array = []
    y_array = []
    y_array_total = []
    for config, config_dic in dic[domain].iteritems():
        del x_array[:]
        del y_array[:]
        del y_array_total[:]
        for num_con, problem_dic in config_dic.iteritems():
            x_array.append(num_con)
            y = 0
            y_total = 0
            for _, coverage in problem_dic.iteritems():
                y += coverage
                y_total += 1
            y_array.append(y)
            y_array_total.append(y_total)

        _, y_array = sort_data_pints(x_array, y_array)
        x_array, y_array_total = sort_data_pints(x_array, y_array_total)

        # write: '--' + markers[i] for line between markers
        line = plt.plot(x_array, y_array, '--' + markers[i], fillstyle=fillstyles[i], markersize=MARKERWIDTH, \
                        linewidth=LINEWIDTH, \
                        markeredgewidth=MEWs[i], \
                        label=config)
        plt.setp(line, color=colors[i])
        i += 1

    line = plt.plot(x_array, y_array_total, '--' + '_', fillstyle='full', markersize=MARKERWIDTH, \
                    linewidth=LINEWIDTH, \
                    markeredgewidth='1', \
                    label='Total')
    plt.setp(line, color='g')

    ax.legend(loc='best', fontsize=legend_FONTSIZE)

    plt.xlabel('#Conn', fontsize=FONTSIZE)
    plt.ylabel('Coverage', fontsize=FONTSIZE)
    plt.xticks(fontsize=FONTSIZE)
    plt.yticks(fontsize=FONTSIZE)
    plt.xscale('log')

    last_x = x_array[-1]
    ticks = [1, 2, 3, 4, 5, 6, 8, 10, 16, 32, 64, 128, 1024, 4096]
    for i in range(len(ticks)):
        tick = ticks[i]
        if tick >= last_x:
            ticks = ticks[0:i+1]
            break

    ax.set_xticks(ticks)
    ax.get_xaxis().set_major_formatter(ScalarFormatter())
    # ax.set_xlim(xmin=0)

    # plt.title('Cost and Reward for Five-eyes vs. DE scenarios 4 and 8')
    ax.grid(color='lightgrey', linestyle='--', linewidth=0.5)

    # ax.yaxis.set_major_formatter(FuncFormatter(lambda x, pos: str(int(x * 100)) + '%'))
    # default_formatter = ax.get_xaxis().get_major_formatter()
    # ax.xaxis.set_major_formatter(FuncFormatter(lambda x, pos: 0 if pos == 2 else default_formatter(x,pos)))

    output_file = domain + ".pdf"
    plt.savefig(output_file, format='pdf', bbox_inches='tight')
    # plt.show()





p = argparse.ArgumentParser(description="")
p.add_argument("--dir", type=str, help="The directory which should be crawled", default=None)
args = p.parse_args(sys.argv[1:])

dir = str(args.dir)
files_in_dir = os.listdir(dir)
#print files_in_dir

all_subdirs = [x[0] for x in os.walk(dir)]
for subdir in all_subdirs:
    #print subdir
    if re.match(".*/\d{5}", subdir) is not None:
        files_in_subdir = os.listdir(subdir)
        for file in files_in_subdir:
            if file.find("properties") != -1:
                parse_properties_file(os.path.join(subdir, file))
                break

#print dic
for domain, _ in dic.iteritems():
    plot_coverage_for_domain(domain)


#for file in filesin_dir:
 #   if file.find("domain") == -1:
  #      modify_problem_file(os.path.join(dir, file), os.path.join(new_dir, file))
   # else:
    #    copyfile(os.path.join(dir, file), os.path.join(new_dir, file))







