import os
import subprocess

from lab.steps import Step
from lab.environments import OracleGridEngineEnvironment, LocalEnvironment
from downward.experiment import DownwardExperiment
from downward.reports.absolute import AbsoluteReport
from downward import suites
from downward.reports.scatter import ScatterPlotReport
from lab.environments import OracleGridEngineEnvironment, LocalEnvironment
from lab import tools
from lab.fetcher import Fetcher

EXPPATH = '/mnt/data_server/torralba/'
REPO = '/mnt/data_server/speicher/sim-pentest-what-if'
 
#ENV = OracleGridEngineEnvironment(queue='all.q@fai11.cs.uni-saarland.de,all.q@fai12.cs.uni-saarland.de,all.q@fai13.cs.uni-saarland.de') 
ENV = OracleGridEngineEnvironment(queue='all.q@fai01.cs.uni-saarland.de,all.q@fai02.cs.uni-saarland.de,all.q@fai03.cs.uni-saarland.de,all.q@fai05.cs.uni-saarland.de,all.q@fai06.cs.uni-saarland.de,all.q@fai07.cs.uni-saarland.de,all.q@fai08.cs.uni-saarland.de') 
#ENV = OracleGridEngineEnvironment(queue='all.q@fai01.cs.uni-saarland.de,all.q@fai02.cs.uni-saarland.de,all.q@fai03.cs.uni-saarland.de,all.q@fai04.cs.uni-saarland.de,all.q@fai05.cs.uni-saarland.de,all.q@fai06.cs.uni-saarland.de,all.q@fai07.cs.uni-saarland.de,all.q@fai08.cs.uni-saarland.de,all.q@fai11.cs.uni-saarland.de,all.q@fai12.cs.uni-saarland.de,all.q@fai13.cs.uni-saarland.de') 

#SUITE = ['gripper:prob01.pddl', 'zenotravel:pfile1']
#SUITE = ['nomystery_C050:p01.pddl', 'nomystery_C050:p02.pddl', 'nomystery_C050:p03.pddl', 'nomystery_C050:p04.pddl', 'nomystery_C050:p05.pddl', 'nomystery_C050:p06.pddl', 'nomystery_C050:p07.pddl', 'nomystery_C050:p08.pddl', 'nomystery_C050:p09.pddl', 'nomystery_C050:p10.pddl', 'nomystery_C050:p11.pddl', 'nomystery_C050:p12.pddl', 'nomystery_C050:p13.pddl', 'nomystery_C050:p14.pddl', 'nomystery_C050:p15.pddl', 'nomystery_C050:p16.pddl', 'nomystery_C050:p17.pddl', 'nomystery_C050:p18.pddl', 'nomystery_C050:p19.pddl', 'nomystery_C050:p20.pddl', 'nomystery_C050:p21.pddl', 'nomystery_C050:p22.pddl', 'nomystery_C050:p23.pddl', 'nomystery_C050:p24.pddl', 'nomystery_C050:p25.pddl']



benchmark_dir = '/mnt/data_server/speicher/sim-pentest-what-if/benchmarks'
SUITE = ['no-mystery-robustness-driving-fuel-rs42-tc1', 'no-mystery-robustness-driving-fuel-rs42-tc10', 'no-mystery-robustness-driving-fuel-rs42-tc12', 'no-mystery-robustness-driving-fuel-rs42-tc16', 'no-mystery-robustness-driving-fuel-rs42-tc2', 'no-mystery-robustness-driving-fuel-rs42-tc25', 'no-mystery-robustness-driving-fuel-rs42-tc3', 'no-mystery-robustness-driving-fuel-rs42-tc32', 'no-mystery-robustness-driving-fuel-rs42-tc4', 'no-mystery-robustness-driving-fuel-rs42-tc5', 'no-mystery-robustness-driving-fuel-rs42-tc6', 'no-mystery-robustness-driving-fuel-rs42-tc8']
#SUITE = ['logistics98-robustness-rs42-tc1', 'logistics98-robustness-rs42-tc10', 'logistics98-robustness-rs42-tc1024', 'logistics98-robustness-rs42-tc12', 'logistics98-robustness-rs42-tc128', 'logistics98-robustness-rs42-tc16', 'logistics98-robustness-rs42-tc2', 'logistics98-robustness-rs42-tc2048', 'logistics98-robustness-rs42-tc25', 'logistics98-robustness-rs42-tc256', 'logistics98-robustness-rs42-tc3', 'logistics98-robustness-rs42-tc32', 'logistics98-robustness-rs42-tc4', 'logistics98-robustness-rs42-tc4096', 'logistics98-robustness-rs42-tc5', 'logistics98-robustness-rs42-tc50', 'logistics98-robustness-rs42-tc512', 'logistics98-robustness-rs42-tc6', 'logistics98-robustness-rs42-tc64', 'logistics98-robustness-rs42-tc8', 'no-mystery-robustness-driving-rs42-tc1', 'no-mystery-robustness-driving-rs42-tc10', 'no-mystery-robustness-driving-rs42-tc12', 'no-mystery-robustness-driving-rs42-tc16', 'no-mystery-robustness-driving-rs42-tc2', 'no-mystery-robustness-driving-rs42-tc25', 'no-mystery-robustness-driving-rs42-tc3', 'no-mystery-robustness-driving-rs42-tc32', 'no-mystery-robustness-driving-rs42-tc4', 'no-mystery-robustness-driving-rs42-tc5', 'no-mystery-robustness-driving-rs42-tc6', 'no-mystery-robustness-driving-rs42-tc8', 'rovers-robustness-driving-rs42-tc1', 'rovers-robustness-driving-rs42-tc10', 'rovers-robustness-driving-rs42-tc12', 'rovers-robustness-driving-rs42-tc128', 'rovers-robustness-driving-rs42-tc16', 'rovers-robustness-driving-rs42-tc2', 'rovers-robustness-driving-rs42-tc25', 'rovers-robustness-driving-rs42-tc256', 'rovers-robustness-driving-rs42-tc3', 'rovers-robustness-driving-rs42-tc32', 'rovers-robustness-driving-rs42-tc4', 'rovers-robustness-driving-rs42-tc5', 'rovers-robustness-driving-rs42-tc50', 'rovers-robustness-driving-rs42-tc512', 'rovers-robustness-driving-rs42-tc6', 'rovers-robustness-driving-rs42-tc64', 'rovers-robustness-driving-rs42-tc8', 'sokoban-opt11-strips-robustness-walls-rs42-tc1', 'sokoban-opt11-strips-robustness-walls-rs42-tc10', 'sokoban-opt11-strips-robustness-walls-rs42-tc12', 'sokoban-opt11-strips-robustness-walls-rs42-tc128', 'sokoban-opt11-strips-robustness-walls-rs42-tc16', 'sokoban-opt11-strips-robustness-walls-rs42-tc2', 'sokoban-opt11-strips-robustness-walls-rs42-tc25', 'sokoban-opt11-strips-robustness-walls-rs42-tc256', 'sokoban-opt11-strips-robustness-walls-rs42-tc3', 'sokoban-opt11-strips-robustness-walls-rs42-tc32', 'sokoban-opt11-strips-robustness-walls-rs42-tc4', 'sokoban-opt11-strips-robustness-walls-rs42-tc5', 'sokoban-opt11-strips-robustness-walls-rs42-tc50', 'sokoban-opt11-strips-robustness-walls-rs42-tc6', 'sokoban-opt11-strips-robustness-walls-rs42-tc64', 'sokoban-opt11-strips-robustness-walls-rs42-tc8', 'tpp-robustness-driving-rs42-tc1', 'tpp-robustness-driving-rs42-tc10', 'tpp-robustness-driving-rs42-tc12', 'tpp-robustness-driving-rs42-tc16', 'tpp-robustness-driving-rs42-tc2', 'tpp-robustness-driving-rs42-tc3', 'tpp-robustness-driving-rs42-tc4', 'tpp-robustness-driving-rs42-tc5', 'tpp-robustness-driving-rs42-tc6', 'tpp-robustness-driving-rs42-tc8', 'transport-opt11-strips-robustness-driving-rs42-tc1', 'transport-opt11-strips-robustness-driving-rs42-tc10', 'transport-opt11-strips-robustness-driving-rs42-tc12', 'transport-opt11-strips-robustness-driving-rs42-tc16', 'transport-opt11-strips-robustness-driving-rs42-tc2', 'transport-opt11-strips-robustness-driving-rs42-tc25', 'transport-opt11-strips-robustness-driving-rs42-tc3', 'transport-opt11-strips-robustness-driving-rs42-tc32', 'transport-opt11-strips-robustness-driving-rs42-tc4', 'transport-opt11-strips-robustness-driving-rs42-tc5', 'transport-opt11-strips-robustness-driving-rs42-tc50', 'transport-opt11-strips-robustness-driving-rs42-tc6', 'transport-opt11-strips-robustness-driving-rs42-tc64', 'transport-opt11-strips-robustness-driving-rs42-tc8', 'transport-opt14-strips-robustness-driving-rs42-tc1', 'transport-opt14-strips-robustness-driving-rs42-tc10', 'transport-opt14-strips-robustness-driving-rs42-tc12', 'transport-opt14-strips-robustness-driving-rs42-tc128', 'transport-opt14-strips-robustness-driving-rs42-tc16', 'transport-opt14-strips-robustness-driving-rs42-tc2', 'transport-opt14-strips-robustness-driving-rs42-tc25', 'transport-opt14-strips-robustness-driving-rs42-tc3', 'transport-opt14-strips-robustness-driving-rs42-tc32', 'transport-opt14-strips-robustness-driving-rs42-tc4', 'transport-opt14-strips-robustness-driving-rs42-tc5', 'transport-opt14-strips-robustness-driving-rs42-tc50', 'transport-opt14-strips-robustness-driving-rs42-tc6', 'transport-opt14-strips-robustness-driving-rs42-tc64', 'transport-opt14-strips-robustness-driving-rs42-tc8', 'visitall-opt14-strips-robustness-driving-rs42-tc1', 'visitall-opt14-strips-robustness-driving-rs42-tc10', 'visitall-opt14-strips-robustness-driving-rs42-tc1024', 'visitall-opt14-strips-robustness-driving-rs42-tc12', 'visitall-opt14-strips-robustness-driving-rs42-tc128', 'visitall-opt14-strips-robustness-driving-rs42-tc16', 'visitall-opt14-strips-robustness-driving-rs42-tc2', 'visitall-opt14-strips-robustness-driving-rs42-tc25', 'visitall-opt14-strips-robustness-driving-rs42-tc256', 'visitall-opt14-strips-robustness-driving-rs42-tc3', 'visitall-opt14-strips-robustness-driving-rs42-tc32', 'visitall-opt14-strips-robustness-driving-rs42-tc4', 'visitall-opt14-strips-robustness-driving-rs42-tc5', 'visitall-opt14-strips-robustness-driving-rs42-tc50', 'visitall-opt14-strips-robustness-driving-rs42-tc512', 'visitall-opt14-strips-robustness-driving-rs42-tc6', 'visitall-opt14-strips-robustness-driving-rs42-tc64', 'visitall-opt14-strips-robustness-driving-rs42-tc8']
#SUITE = suites.suite_ipc14_sat_strips() + suites.suite_ipc11_sat() + ['tpp', 'no-mystery', 'rovers']
#SUITE = ['childsnack-opt14-strips:child-snack_pfile01.pddl', 'childsnack-opt14-strips:child-snack_pfile01-2.pddl', 'childsnack-opt14-strips:child-snack_pfile02.pddl', 'childsnack-opt14-strips:child-snack_pfile02-2.pddl']

LIMITS={'search_time': 1800,  'search_memory' : 4096, 'translate_memory': 4096, 'preprocess_memory': 4096}

ATTRIBUTES = ['error', 'search_time', 'memory', 'evaluations', 'expansions', 'initial_h_value', 'coverage', 'plan_length', 'cost', 'total_time']

exp = DownwardExperiment(path=EXPPATH, repo=REPO, environment=ENV, limits=LIMITS, )
exp.add_suite(SUITE, benchmark_dir)

PARSER = 'parser.py'
exp.add_search_parser(PARSER) 

# Make a report containing absolute numbers (this is the most common report).
report = os.path.join(exp.eval_dir, 'report.html')
exp.add_report(AbsoluteReport(attributes=ATTRIBUTES), outfile=report)

# Parse the commandline and show or run experiment steps.
exp()
