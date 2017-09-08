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
legend_FONTSIZE = 17
LINEWIDTH = 1
MARKERWIDTH = 24
MEW = 2

colors = ['b', 'r', 'g', 'k', 'y', 'm', 'c', 'C0', 'C1', 'C2', 'C3', 'C4']
markers = ['x', 's', '+', '.', 'd', '*']
fillstyles = ['full', 'none', 'full', 'none', 'none', 'none']
MEWs = ['1', '1', '1', '1', '1', '1']


dic = {}
num_instances_of_domain = {}

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
    search_returncode = content['search_returncode']
    config = content['config_nick']
    if (domain.find("sokoban") != -1 or domain.find("no-mystery") != -1) and coverage == 0 and search_returncode == 0:
        print "HACK! Set coverage to 1 because of 0 return code for " + domain_and_num_con + "/" + problem
        coverage = 1

    #print domain_and_num_con
    #print domain
    #print num_con
    #print problem
    #print coverage
    #print config
    if domain not in dic:
        dic[domain] = dict()
    if config not in dic[domain]:
        dic[domain][config] = dict()
    if num_con not in dic[domain][config]:
        dic[domain][config][num_con] = dict()
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

def prettify_config (config):
    config_mapping = {'IDS with DEADPDB, LM-cut, w/o AUBP': 'IDS FSP + SSS',
                      'IDS with DEADPDB, FF, w/o POR PAPA AUBP': 'IDS Baseline',
                      'IDS with DEADPDB, LM-cut': 'FSP + SSS + LSP',
                      'IDS with DEADPDB, FF, w/o AUBP': 'IDS FSP + SSS',
                      'IDS with DEADPDB, FF, w/o PAPA': 'SSS + LSP',
                      'IDS with DEADPDB, LM-cut, w/o PAPA': 'SSS + LSP',
                      'IDS with DEADPDB, FF': 'FSP + SSS + LSP',
                      'IDS with DEADPDB, FF, and sorting fix ops, w/o POR': 'FSP + LSP',
                      'IDS with DEADPDB, LM-cut, w/o POR': 'FSP + LSP',
                      'IDS with DEADPDB, LM-cut, w/o POR PAPA AUBP': 'IDS Baseline',
                      'DFS with DEADPDB, FF, w/o POR PAPA AUBP': 'Baseline',
                      'DFS with DEADPDB, LM-cut, w/o POR PAPA AUBP': 'Baseline',
                      'DFS with DEADPDB, LM-cut, w/o AUBP': 'FSP + SSS',
                      'DFS with DEADPDB, FF, w/o AUBP': 'FSP + SSS'}
    return config_mapping[config]

def compare_config_iteritems (items1, items2):
    place_in_legend_for_prettified_config = {'Baseline': 0,
                                             'IDS Baseline': 1,
                                             'FSP + SSS': 2,
                                             'IDS FSP + SSS': 3,
                                             'FSP + LSP': 4,
                                             'SSS + LSP': 5,
                                             'FSP + SSS + LSP': 10
                                             }

    if place_in_legend_for_prettified_config[prettify_config(items1[0])] < place_in_legend_for_prettified_config[prettify_config(items2[0])]:
        return -1
    elif place_in_legend_for_prettified_config[prettify_config(items1[0])] > place_in_legend_for_prettified_config[prettify_config(items2[0])]:
        return 1
    else:
        return 0


def plot_coverage_for_domain(domain):
    fig, ax = plt.subplots(figsize=(13, 6))
    # plt.style.use('grayscale')

    abs_num_total_instances = 0.0
    #for _, coverage in dic[domain][dic[domain].keys()[0]][1].iteritems():
    #   abs_num_total_instances += 1.0

    x_arrays = []
    y_arrays = []
    y_total_arrays = []
    first_zero_coverage_indezes = []
    configs = []
    config_iteritems = sorted(dic[domain].iteritems(), cmp=compare_config_iteritems)
    for config, config_dic in config_iteritems:
        if interesting_configs is not None and config not in interesting_configs:
            continue

        x_array = []
        y_array = []
        y_array_total = []
        for num_con, problem_dic in config_dic.iteritems():
            x_array.append(num_con)
            y = 0.0
            y_total = 0.0
            for _, coverage in problem_dic.iteritems():
                y += coverage
                y_total += 1.0
            y_array.append(y/y_total)
            #y_array_total.append(y_total/abs_num_total_instances)

        _, y_array = sort_data_pints(x_array, y_array)
        x_array, y_array_total = sort_data_pints(x_array, y_array_total)

        #assert (y_array_total[0] == 1.0)

        first_zero_coverage_index = len(y_array) - 1
        for j in range(len(y_array)):
            if first_zero_coverage_index == len(y_array) - 1  and y_array[j] == 0:
                first_zero_coverage_index = j
            elif first_zero_coverage_index != len(y_array) - 1:
                if y_array[j] != 0:
                    first_zero_coverage_index = len(y_array) - 1
                    print "first_zero_coverage_index resetted for " + domain + " " + config

        x_arrays.append(x_array)
        y_arrays.append(y_array)
        y_total_arrays.append(y_array_total)
        first_zero_coverage_indezes.append(first_zero_coverage_index)
        configs.append(config)


    max_first_zero_coverage_index = 0
    for i in range(len(first_zero_coverage_indezes)):
        max_first_zero_coverage_index = max(max_first_zero_coverage_index, first_zero_coverage_indezes[i])

    for i in range(len(x_arrays)):
        x_array = x_arrays[i]
        y_array = y_arrays[i]
        # write: '--' + markers[i] for line between markers
        # additional marker options: , fillstyle=fillstyles[i], markersize=MARKERWIDTH, markeredgewidth=MEWs[i]
        line = plt.plot(x_array[:max_first_zero_coverage_index + 1], y_array[:max_first_zero_coverage_index + 1], '--' + markers[i],
                    fillstyle=fillstyles[i], markersize=MARKERWIDTH, markeredgewidth=MEWs[i], \
                    linewidth=LINEWIDTH, \
                    label=prettify_config(configs[i]))
        plt.setp(line, color=colors[i])


    #line = plt.plot(x_array, y_array_total, '--' + '_', fillstyle='full', markersize=MARKERWIDTH, \
    #                linewidth=LINEWIDTH, \
    #                markeredgewidth='1', \
    #                label='Total')
    #plt.setp(line, color='g')

    ax.legend(loc='best', fontsize=legend_FONTSIZE)

    #if domain == "pentesting-robustness-rs42":
    #    plt.xlabel('Fix actions', fontsize=FONTSIZE)
    #elif domain.find("sokoban-opt11-strips-robustness") != -1:
    #    plt.xlabel('Walls', fontsize=FONTSIZE)
    #else:
    #    plt.xlabel('Connections', fontsize=FONTSIZE)
    #plt.ylabel('Coverage', fontsize=FONTSIZE)
    plt.xticks(fontsize=FONTSIZE)
    plt.yticks(fontsize=FONTSIZE)
    plt.xscale('log')

    last_x = x_arrays[0][:max_first_zero_coverage_index+1][-1]
    ticks = [1, 2, 3, 4, 5, 6, 8, 10, 16, 25, 32, 50, 64, 128, 256, 1024, 4096]
    for i in range(len(ticks)):
        tick = ticks[i]
        if tick >= last_x:
            ticks = ticks[0:i+1]
            break

    ax.set_xticks(ticks)
    ax.get_xaxis().set_major_formatter(ScalarFormatter())
    ax.set_xlim(xmin=0.85)

    # plt.title('Cost and Reward for Five-eyes vs. DE scenarios 4 and 8')
    ax.grid(color='lightgrey', linestyle='--', linewidth=0.5)

    ax.set_ylim(ymin=-0.05, ymax=1.05)
    ax.yaxis.set_major_formatter(FuncFormatter(lambda x, pos: str(int(x * 100)) + '%'))
    # default_formatter = ax.get_xaxis().get_major_formatter()
    # ax.xaxis.set_major_formatter(FuncFormatter(lambda x, pos: 0 if pos == 2 else default_formatter(x,pos)))

    output_file = domain + args.name_suffix + ".pdf"
    plt.savefig(output_file, format='pdf', bbox_inches='tight')
    # plt.show()





p = argparse.ArgumentParser(description="")
p.add_argument("--dir", nargs='+', type=str, help="The directories which should be crawled", default=None)
p.add_argument("--configs", nargs='+', type=str, help="The interesting configs which you want to plot", default=None)
p.add_argument("--domains", nargs='+', type=str, help="The interesting domains which you want to plot", default=None)
p.add_argument("--name-suffix", type=str, help="The output filename suffix", default="")
args = p.parse_args(sys.argv[1:])

interesting_configs = args.configs
interesting_domains = args.domains

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
                    parse_properties_file(os.path.join(subdir, file))
                    break

#print dic
for domain, _ in dic.iteritems():
    if interesting_domains == None or domain in interesting_domains:
        plot_coverage_for_domain(domain)


#for file in filesin_dir:
 #   if file.find("domain") == -1:
  #      modify_problem_file(os.path.join(dir, file), os.path.join(new_dir, file))
   # else:
    #    copyfile(os.path.join(dir, file), os.path.join(new_dir, file))







