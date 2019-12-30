#!/usr/bin/env python
# encoding: utf-8

import sys
sys.append('./config')

import errno
import os

from subprocess import call

import configs

def get_queue(machines):
    if (machines == 'old_servers'):
        return "OracleGridEngineEnvironment(queue='all.q@fai01.cs.uni-saarland.de,all.q@fai02.cs.uni-saarland.de,all.q@fai03.cs.uni-saarland.de,all.q@fai04.cs.uni-saarland.de,all.q@fai05.cs.uni-saarland.de,all.q@fai06.cs.uni-saarland.de,all.q@fai07.cs.uni-saarland.de,all.q@fai08.cs.uni-saarland.de')"
    elif (machines == 'new_servers'):
        return "OracleGridEngineEnvironment(queue='all.q@fai11.cs.uni-saarland.de,all.q@fai12.cs.uni-saarland.de,all.q@fai13.cs.uni-saarland.de,all.q@fai14.cs.uni-saarland.de,all.q@fai14.cs.uni-saarland.de')"
    elif (machines == 'all_servers'):
        return "OracleGridEngineEnvironment(queue='all.q@fai01.cs.uni-saarland.de,all.q@fai02.cs.uni-saarland.de,all.q@fai03.cs.uni-saarland.de,all.q@fai04.cs.uni-saarland.de,all.q@fai05.cs.uni-saarland.de,all.q@fai06.cs.uni-saarland.de,all.q@fai07.cs.uni-saarland.de,all.q@fai08.cs.uni-saarland.de,all.q@fai11.cs.uni-saarland.de,all.q@fai12.cs.uni-saarland.de,all.q@fai13.cs.uni-saarland.de,all.q@fai14.cs.uni-saarland.de')"
    else:
        print("ERROR: please specify on which machines the experiments should be running!", machines)
        exit(1)


def get_script(config):
    QUEUES = get_queue(config.machines)

    SUITE = ['logistics98-robustness-rs42-tc1', 'logistics98-robustness-rs42-tc10', 'logistics98-robustness-rs42-tc1024', 'logistics98-robustness-rs42-tc12', 'logistics98-robustness-rs42-tc128', 'logistics98-robustness-rs42-tc16', 'logistics98-robustness-rs42-tc2', 'logistics98-robustness-rs42-tc2048', 'logistics98-robustness-rs42-tc25', 'logistics98-robustness-rs42-tc256', 'logistics98-robustness-rs42-tc3', 'logistics98-robustness-rs42-tc32', 'logistics98-robustness-rs42-tc4', 'logistics98-robustness-rs42-tc4096', 'logistics98-robustness-rs42-tc5', 'logistics98-robustness-rs42-tc50', 'logistics98-robustness-rs42-tc512', 'logistics98-robustness-rs42-tc6', 'logistics98-robustness-rs42-tc64', 'logistics98-robustness-rs42-tc8', 'no-mystery-robustness-driving-rs42-tc1', 'no-mystery-robustness-driving-rs42-tc10', 'no-mystery-robustness-driving-rs42-tc12', 'no-mystery-robustness-driving-rs42-tc16', 'no-mystery-robustness-driving-rs42-tc2', 'no-mystery-robustness-driving-rs42-tc25', 'no-mystery-robustness-driving-rs42-tc3', 'no-mystery-robustness-driving-rs42-tc32', 'no-mystery-robustness-driving-rs42-tc4', 'no-mystery-robustness-driving-rs42-tc5', 'no-mystery-robustness-driving-rs42-tc6', 'no-mystery-robustness-driving-rs42-tc8', 'rovers-robustness-driving-rs42-tc1', 'rovers-robustness-driving-rs42-tc10', 'rovers-robustness-driving-rs42-tc12', 'rovers-robustness-driving-rs42-tc128', 'rovers-robustness-driving-rs42-tc16', 'rovers-robustness-driving-rs42-tc2', 'rovers-robustness-driving-rs42-tc25', 'rovers-robustness-driving-rs42-tc256', 'rovers-robustness-driving-rs42-tc3', 'rovers-robustness-driving-rs42-tc32', 'rovers-robustness-driving-rs42-tc4', 'rovers-robustness-driving-rs42-tc5', 'rovers-robustness-driving-rs42-tc50', 'rovers-robustness-driving-rs42-tc512', 'rovers-robustness-driving-rs42-tc6', 'rovers-robustness-driving-rs42-tc64', 'rovers-robustness-driving-rs42-tc8', 'sokoban-opt11-strips-robustness-walls-rs42-tc1', 'sokoban-opt11-strips-robustness-walls-rs42-tc10', 'sokoban-opt11-strips-robustness-walls-rs42-tc12', 'sokoban-opt11-strips-robustness-walls-rs42-tc128', 'sokoban-opt11-strips-robustness-walls-rs42-tc16', 'sokoban-opt11-strips-robustness-walls-rs42-tc2', 'sokoban-opt11-strips-robustness-walls-rs42-tc25', 'sokoban-opt11-strips-robustness-walls-rs42-tc256', 'sokoban-opt11-strips-robustness-walls-rs42-tc3', 'sokoban-opt11-strips-robustness-walls-rs42-tc32', 'sokoban-opt11-strips-robustness-walls-rs42-tc4', 'sokoban-opt11-strips-robustness-walls-rs42-tc5', 'sokoban-opt11-strips-robustness-walls-rs42-tc50', 'sokoban-opt11-strips-robustness-walls-rs42-tc6', 'sokoban-opt11-strips-robustness-walls-rs42-tc64', 'sokoban-opt11-strips-robustness-walls-rs42-tc8', 'tpp-robustness-driving-rs42-tc1', 'tpp-robustness-driving-rs42-tc10', 'tpp-robustness-driving-rs42-tc12', 'tpp-robustness-driving-rs42-tc16', 'tpp-robustness-driving-rs42-tc2', 'tpp-robustness-driving-rs42-tc3', 'tpp-robustness-driving-rs42-tc4', 'tpp-robustness-driving-rs42-tc5', 'tpp-robustness-driving-rs42-tc6', 'tpp-robustness-driving-rs42-tc8', 'transport-opt11-strips-robustness-driving-rs42-tc1', 'transport-opt11-strips-robustness-driving-rs42-tc10', 'transport-opt11-strips-robustness-driving-rs42-tc12', 'transport-opt11-strips-robustness-driving-rs42-tc16', 'transport-opt11-strips-robustness-driving-rs42-tc2', 'transport-opt11-strips-robustness-driving-rs42-tc25', 'transport-opt11-strips-robustness-driving-rs42-tc3', 'transport-opt11-strips-robustness-driving-rs42-tc32', 'transport-opt11-strips-robustness-driving-rs42-tc4', 'transport-opt11-strips-robustness-driving-rs42-tc5', 'transport-opt11-strips-robustness-driving-rs42-tc50', 'transport-opt11-strips-robustness-driving-rs42-tc6', 'transport-opt11-strips-robustness-driving-rs42-tc64', 'transport-opt11-strips-robustness-driving-rs42-tc8', 'transport-opt14-strips-robustness-driving-rs42-tc1', 'transport-opt14-strips-robustness-driving-rs42-tc10', 'transport-opt14-strips-robustness-driving-rs42-tc12', 'transport-opt14-strips-robustness-driving-rs42-tc128', 'transport-opt14-strips-robustness-driving-rs42-tc16', 'transport-opt14-strips-robustness-driving-rs42-tc2', 'transport-opt14-strips-robustness-driving-rs42-tc25', 'transport-opt14-strips-robustness-driving-rs42-tc3', 'transport-opt14-strips-robustness-driving-rs42-tc32', 'transport-opt14-strips-robustness-driving-rs42-tc4', 'transport-opt14-strips-robustness-driving-rs42-tc5', 'transport-opt14-strips-robustness-driving-rs42-tc50', 'transport-opt14-strips-robustness-driving-rs42-tc6', 'transport-opt14-strips-robustness-driving-rs42-tc64', 'transport-opt14-strips-robustness-driving-rs42-tc8', 'visitall-opt14-strips-robustness-driving-rs42-tc1', 'visitall-opt14-strips-robustness-driving-rs42-tc10', 'visitall-opt14-strips-robustness-driving-rs42-tc1024', 'visitall-opt14-strips-robustness-driving-rs42-tc12', 'visitall-opt14-strips-robustness-driving-rs42-tc128', 'visitall-opt14-strips-robustness-driving-rs42-tc16', 'visitall-opt14-strips-robustness-driving-rs42-tc2', 'visitall-opt14-strips-robustness-driving-rs42-tc25', 'visitall-opt14-strips-robustness-driving-rs42-tc256', 'visitall-opt14-strips-robustness-driving-rs42-tc3', 'visitall-opt14-strips-robustness-driving-rs42-tc32', 'visitall-opt14-strips-robustness-driving-rs42-tc4', 'visitall-opt14-strips-robustness-driving-rs42-tc5', 'visitall-opt14-strips-robustness-driving-rs42-tc50', 'visitall-opt14-strips-robustness-driving-rs42-tc512', 'visitall-opt14-strips-robustness-driving-rs42-tc6', 'visitall-opt14-strips-robustness-driving-rs42-tc64', 'visitall-opt14-strips-robustness-driving-rs42-tc8']

    return """#! /usr/bin/env python

import os
import subprocess

import suites

from lab.steps import Step
from downward.checkouts import Translator, Preprocessor, Planner
from downward.experiment import DownwardExperiment
from downward.reports.absolute import AbsoluteReport

from lab.environments import OracleGridEngineEnvironment

REVISION = '" + config.revision + "'

EXPPATH = '/mnt/data_server/torralba/stackelberg/results/" + config.machines + "/' + REVISION  + '/" + config.folder + "/'

REPO = '/mnt/data_server/torralba/dominance-journal/stackelberg/sim-pentest-what-if'

ENV = " + QUEUES + "

ATTRIBUTES = ['search_time', 'reopened', 'memory', 'evaluations', 'total_time', 'expansions', 'error', 'generated', 'initial_h_values', 'coverage', 'plan_length', 'max_number_dups', 'cost']

LIMITS={'search_time': 1800,  'search_memory' : 4096}

COMBINATIONS = [(Translator(repo=REPO), Preprocessor(repo=REPO), Planner(repo=REPO, rev=REVISION))]

exp = DownwardExperiment(path=EXPPATH, repo=REPO, environment=ENV, combinations=COMBINATIONS, limits=LIMITS, cache_dir='/mnt/data_server/torralba/stackelberg/lab-data/')

exp.add_search_parser(REPO + '/lab_parser.py')

exp.add_config('" + config.nick + "' + REVISION, " + str(config.config) + ")

exp.add_suite(" + SUITE + ", benchmark_dir='/mnt/data_server/torralba/stackelberg/sim-pentest-what-if/benchmarks')


def remove_work_tag(run):
    \"\"\"Remove \"WORK-\" from the configs.\"\"\"
    config = run['config']
    config = config[5:] if config.startswith('WORK-') else config
    # Shorten long config name.
    config = config.replace('downward-', '')
    run['config'] = config
    return run

# Make a report containing absolute numbers (this is the most common report).
exp.add_report(AbsoluteReport(attributes=ATTRIBUTES), '/mnt/data_server/torralba/stackelberg/reports/" + config.folder + "-rev=' + REVISION)

# Parse the commandline and show or run experiment steps.
exp()

"""


if __name__ == '__main__':
    if (len(sys.argv) < 3):
        print ("please specify experiment and output folder")
        print ("\n".join(configs.CONFIGS.keys()))
        
        exit()
    
    experiment = sys.argv[1]
    folder = sys.argv[2]
  
    if (not folder.endswith('/')):
        folder = folder + '/'
    print "writing lab scripts to this subfolder: " + folder
    try:
        os.makedirs(folder)
    except OSError as exc:
        if exc.errno == errno.EEXIST and os.path.isdir(folder):
            pass
        else: raise
    CONFIGS_TO_PROCESS = configs.get_configs(experiment)
    for config in CONFIGS_TO_PROCESS:
        print config
        data = get_script(config)

        EXPPATH = '/mnt/data_server/torralba/stackelberg/results/{}/{}/{}'.format(config.machines, config.revision, config.folder)
        if os.path.isdir(EXPPATH):
            continue
        name = folder + config.folder + ".py"
        with open(name, "w") as file:
            file.write(data)
            call(["chmod", "+x", name])
            print "successfully written " + name 
