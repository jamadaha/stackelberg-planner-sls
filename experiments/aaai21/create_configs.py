#!/usr/bin/env python2
# encoding: utf-8

import sys
sys.path.append('./configs')

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
    SUITE = config.SUITE
    
    return """#! /usr/bin/env python2

import os
import subprocess

import suites

from lab.steps import Step
from downward.checkouts import TranslatorGit, PreprocessorGit, PlannerGit
from downward.experiment import DownwardExperiment
from downward.reports.absolute import AbsoluteReport

from lab.environments import OracleGridEngineEnvironment

REVISION = '{config.revision}'
REVISION_PREPROCESS = '{config.preprocess_revision}'

EXPPATH = '/mnt/data_server/torralba/stackelberg/results/{config.machines}/{config.revision}/{config.folder}'

REPO = '/mnt/data_server/torralba/stackelberg/sim-pentest-what-if'

ENV = {QUEUES}

ATTRIBUTES = ['search_time', 'memory', 'total_time', 'error', 'coverage', 'pareto_frontier_size', 'follower_time', 'leader_time']

LIMITS={{'search_time': 1800,  'search_memory' : 4096}}

COMBINATIONS = [(TranslatorGit(repo=REPO, rev=REVISION_PREPROCESS), PreprocessorGit(repo=REPO, rev=REVISION_PREPROCESS), PlannerGit(repo=REPO, rev=REVISION))]

SUITE = {SUITE}
exp = DownwardExperiment(path=EXPPATH, repo=REPO, environment=ENV, combinations=COMBINATIONS, limits=LIMITS, cache_dir='/mnt/data_server/torralba/stackelberg/lab-data/')

exp.add_search_parser(REPO + '/lab_parser.py')

exp.add_config('{config.nick}' + REVISION, {config.config})

exp.add_suite(SUITE, benchmark_dir='/mnt/data_server/torralba/stackelberg/sim-pentest-what-if/new-benchmarks/{config.benchmarks_dir}/')


def remove_work_tag(run):
    config = run['config']
    config = config[5:] if config.startswith('WORK-') else config
    # Shorten long config name.
    config = config.replace('downward-', '')
    run['config'] = config
    return run

# Make a report containing absolute numbers (this is the most common report).
exp.add_report(AbsoluteReport(attributes=ATTRIBUTES), '/mnt/data_server/torralba/stackelberg/reports/{config.folder}-rev=' + REVISION)

# Parse the commandline and show or run experiment steps.
exp()

""".format(**locals())


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
        else:
            raise
        
    CONFIGS_TO_PROCESS = configs.get_configs(experiment)
    for config in CONFIGS_TO_PROCESS:
        data = get_script(config)

        EXPPATH = '/mnt/data_server/torralba/stackelberg/results/{}/{}/{}'.format(config.machines, config.revision, config.folder)
        if os.path.isdir(EXPPATH):
            continue
        name = folder + config.folder + ".py"
        with open(name, "w") as file:
            file.write(data)
            call(["chmod", "+x", name])
            print "successfully written " + name 
