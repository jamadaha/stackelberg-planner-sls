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

    SUITE = ['logistics-rs42-tc1', 
             'logistics-rs42-tc10',
             'logistics-rs42-tc12',
             'logistics-rs42-tc128',
             'logistics-rs42-tc16',
             'logistics-rs42-tc2',
             'logistics-rs42-tc25',
             'logistics-rs42-tc3',
             'logistics-rs42-tc32',
             'logistics-rs42-tc4',
             'logistics-rs42-tc5',
             'logistics-rs42-tc50',
             'logistics-rs42-tc6',
             'logistics-rs42-tc64',
             'logistics-rs42-tc8',
             'logistics-rs42-tcall',
             'nomystery-rs42-tc1',
             'nomystery-rs42-tc10',
             'nomystery-rs42-tc12',
             'nomystery-rs42-tc128',
             'nomystery-rs42-tc16',
             'nomystery-rs42-tc2',
             'nomystery-rs42-tc25',
             'nomystery-rs42-tc3',
             'nomystery-rs42-tc32',
             'nomystery-rs42-tc4',
             'nomystery-rs42-tc5',
             'nomystery-rs42-tc50',
             'nomystery-rs42-tc6',
             'nomystery-rs42-tc64',
             'nomystery-rs42-tc8',
             'nomystery-rs42-tcall',
             'rovers-rs42-tc1',
             'rovers-rs42-tc10',
             'rovers-rs42-tc12',
             'rovers-rs42-tc16',
             'rovers-rs42-tc2',
             'rovers-rs42-tc25',
             'rovers-rs42-tc3',
             'rovers-rs42-tc32',
             'rovers-rs42-tc4',
             'rovers-rs42-tc5',
             'rovers-rs42-tc50',
             'rovers-rs42-tc6',
             'rovers-rs42-tc8',
             'rovers-rs42-tcall',
             'tpp-rs42-tc1',
             'tpp-rs42-tc10',
             'tpp-rs42-tc12',
             'tpp-rs42-tc128',
             'tpp-rs42-tc16',
             'tpp-rs42-tc2',
             'tpp-rs42-tc25',
             'tpp-rs42-tc256',
             'tpp-rs42-tc3',
             'tpp-rs42-tc32',
             'tpp-rs42-tc4',
             'tpp-rs42-tc5',
             'tpp-rs42-tc50',
             'tpp-rs42-tc6',
             'tpp-rs42-tc64',
             'tpp-rs42-tc8',
             'tpp-rs42-tcall',
             'transport-rs42-tc1',
             'transport-rs42-tc10',
             'transport-rs42-tc12',
             'transport-rs42-tc128',
             'transport-rs42-tc16',
             'transport-rs42-tc2',
             'transport-rs42-tc25',
             'transport-rs42-tc256',
             'transport-rs42-tc3',
             'transport-rs42-tc32',
             'transport-rs42-tc4',
             'transport-rs42-tc5',
             'transport-rs42-tc50',
             'transport-rs42-tc6',
             'transport-rs42-tc64',
             'transport-rs42-tc8',
             'transport-rs42-tcall',
             'visitall-rs42-tc1',
             'visitall-rs42-tc10',
             'visitall-rs42-tc12',
             'visitall-rs42-tc128',
             'visitall-rs42-tc16',
             'visitall-rs42-tc2',
             'visitall-rs42-tc25',
             'visitall-rs42-tc3',
             'visitall-rs42-tc32',
             'visitall-rs42-tc4',
             'visitall-rs42-tc5',
             'visitall-rs42-tc50',
             'visitall-rs42-tc6',
             'visitall-rs42-tc64',
             'visitall-rs42-tc8',
             'visitall-rs42-tcall',
    ]
    
    SUITE = [domain for domain in SUITE if not domain.endswith("tc1") and not domain.endswith("tc3") and not domain.endswith("tc5") and not domain.endswith("tc12") and not "transport-opt14" in domain and not domain.endswith("tc25") and not domain.endswith("tc50") and not domain.endswith("tc6") and not domain.endswith("tc10")]

    # SUITE = ["logistics-rs42-tcall", "nomystery-rs42-tcall", "rovers-rs42-tcall", "tpp-rs42-tcall", "transport-rs42-tcall", "visitall-rs42-tcall"] 
    
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

exp.add_suite(SUITE, benchmark_dir='/mnt/data_server/torralba/stackelberg/sim-pentest-what-if/new-benchmarks')


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
