#! /usr/bin/env python
# -*- coding: utf-8 -*-

import itertools
import logging
import numpy
import os

from collections import defaultdict

from downward.experiment import FastDownwardExperiment
from downward.reports import PlanningReport
from downward.reports.absolute import AbsoluteReport
from downward.reports.compare import ComparativeReport
from downward.reports.scatter import ScatterPlotReport

from lab.reports import Attribute, geometric_mean, arithmetic_mean

from lab import tools

from common_setup import IssueExperiment

exp = FastDownwardExperiment()

REVISIONS = [
    '1b546175d83d134245d3c4099014e776dd9fccb8',
    '6f1b5abf91f0b5a9f7dace3fea8ffbedcff3c7dc',
    '92845ada3fd61a733f99702ffcfff2b3b99260f0',
    '8afa96ef3ce86a6b39ea031ac396f7b732dc43a5',
    'f503350c800543392b4054ca91b074252776dd34',
    '0ab0299b306776810922f24fe1c65df7bf82d3f8',
    'adeb756f50388450a3c15e295d7d45abd98812fe',
    'd059552e393f05e01d52bb8bd880873acf4dce78',
    'aaai18ipc',
    'aaai21ipc',
    'fixed',
    '3cd44a8df4b9332f3658295ccd85430f71ed410e',
    'e9a4dbefd3848636f38b5a38c2a6a56173a73bfa'
]

def rename_algorithm_and_domain(run):
    algo = run['config']

    dom = run['domain']
    if dom in ["aaai21-rovers-driving", "aaai21-logistics-driving"]:
        return False

    if "aaai18-pipesworld-notankage" in dom:
        return False
    
    if "ss-lmcut-pdbs" in algo:
        return False
    # if "aaai18" in dom:
    #     return False
    
    for rev in REVISIONS:
        algo = algo.replace('{}'.format(rev), '')

    dom = dom.replace("-robustness", "")
    dom = dom.replace("-rs42", "")

    if "-tc" in dom:
        parts = dom.split("-tc")

        
        if "-" in parts[1]:
            tcpart = "-tc" + parts[1].split("-")[0]
            dompart = parts[1].split("-")[1]
        else:
            tcpart = "-tc" + parts[1].split("-")[0]
            dompart = ""

            
        run["problem"] += tcpart
        dom = parts[0] + dompart


    if "-soft" in algo:
        algo = algo.replace("-soft", "")
        dom = dom + "-soft"

    algo_parts = [x for x in algo.split("-") if x]
    algo = "-".join(algo_parts) 
    
    run['algorithm'] = algo
    run['config'] = algo
    run['domain'] = dom

    if "coverage" not in run:
        print ("Warning: Run without coverage is excluded: {} {} {}".format(algo, dom, run["problem"]))
        return False
    return run




def add_histogram(run):
    for a in [3, 5, 10, 20, 50, 100]:
        run['histogram_follower_searches_{}'.format(a)] = 1 if 'total_follower_searches' in run and run['total_follower_searches'] >= a else 0
    
    return run

def correct_statistics(run):
    if 'optimally_solved_subproblems' in run and not 'optimal_solver_searches' in run:
        run['optimal_solver_searches'] = run['optimally_solved_subproblems']

    if 'optimally_solved_subproblems' in run and not 'total_follower_searches' in run:
        run['total_follower_searches'] = run['optimally_solved_subproblems']

    return run

def add_component_options(run):
    run["component_options"] = run["commandline_config"]
    return run

exp.add_fetcher('data/all',filter=[rename_algorithm_and_domain, correct_statistics, add_histogram, add_component_options ])


attributes = ['search_time', 'memory', 'total_time', 'error', 'coverage', 'pareto_frontier_size', 'follower_time', 'optimally_solved_subproblems', 'total_follower_searches', 'optimal_solver_searches'] + ['histogram_follower_searches_{}'.format(a) for a in [3, 5, 10, 20, 50, 100] ]
# attributes.extend(extra_attributes)


## HTML reports

exp.add_report(AbsoluteReport(attributes=['total_time', 'coverage', 'optimal_solver_searches', 'follower_time']))

# exp.add_report(AbsoluteReport(attributes=['total_time', 'coverage', 'optimal_solver_searches'], filter_algorithm=["ss-sbd", "baseline-lmcut", "ss-sbd-ubreuse", "ss-sbd-ubreuse-cbff-1s", "ss-sbd-ubreuse-cbff-10s", "ss-sbd-ubreuse-cbffpr-1s", "ss-sbd-ubreuse-cbffpr-5s", "ss-sbd-ubreuse-cbffpr-10s", "ss-sbd-cbff-1s", "ss-sbd-cbff-10s", "ss-sbd-cbffpr-10s" ]))

exp.add_report(AbsoluteReport(attributes=['total_time', 'coverage', 'optimal_solver_searches'], filter_algorithm=["ss-sbd", "ss-sbd-cbff-1s", "ss-sbd-cbff-10s", "ss-sbd-cbffpr-10s" ]), outfile='report-sbd-cbff.html')

exp.add_report(AbsoluteReport(attributes=['total_time', 'coverage', 'optimal_solver_searches'], filter_algorithm=["ss-sbd-ubreuse", "ss-sbd-ubreuse-cbff-1s", "ss-sbd-ubreuse-cbff-10s", "ss-sbd-ubreuse-cbffpr-1s", "ss-sbd-ubreuse-cbffpr-5s", "ss-sbd-ubreuse-cbffpr-10s"]),  outfile='report-sbd-ubreuse-cbff.html')

exp.add_report(AbsoluteReport(attributes=['total_time', 'coverage', 'optimal_solver_searches', 'follower_time', 'optimally_solved_subproblems'], filter_algorithm=["ss-sbd-ubreuse", "ss-sbd-ubreuse-regress"]), outfile='report-sbd-regress.html')

exp.add_report(AbsoluteReport(attributes=['total_time', 'coverage', 'optimal_solver_searches', 'follower_time'], filter_algorithm=["baseline-sbd", "ss-sbd", "ss-sbd-ubreuse", "ss-sbd-ubreuse-cbff-1s", 'ss-sbd-up-ubreuse-tlim']), outfile='report-sbd.html')
exp.add_report(AbsoluteReport(attributes=['total_time', 'coverage', 'optimal_solver_searches', 'follower_time'], filter_algorithm=["baseline-lmcut", "ss-lmcut", "ss-sbd-lmcut", 'ss-lmcut-ubreuse']), outfile='report-lmcut.html')

exp.add_report(AbsoluteReport(attributes=['total_time', 'coverage', 'optimal_solver_searches', 'follower_time'], filter_algorithm=["original-lmcut-pdbs","baseline-lmcut", "ss-lmcut", "ss-sbd-lmcut", 'ss-lmcut-ubreuse', "baseline-sbd", "ss-sbd", "ss-sbd-ubreuse", "ss-sbd-ubreuse-cbff-1s", 'ss-sbd-up-ubreuse-tlim']), outfile='report-lmcut-sbd.html')



# exp.add_report(AbsoluteReport(attributes=['search_time', 'memory', 'total_time', 'error', 'coverage', 'pareto_frontier_size', 'follower_time', 'optimally_solved_subproblems', 'total_follower_searches', 'optimal_solver_searches']))

### Latex reports

## expansion plots for bisim



# for atr in ['optimally_solved_subproblems', "search_time", 'total_time', 'total_follower_searches', 'follower_time']:
#     for alg1, alg2 in [('ss-sbd','ss-sbd-ubreuse'), ('ss-lmcut','ss-lmcut-ubreuse')]: # ('ss-sbd','ss-sbd-up'), ('ss-sbd', 'ss-sbd-up-ubreuse-tlim')
#         outf = os.path.join(exp.eval_dir, '{}-{}-vs-{}'.format(atr, alg1, alg2))
#         if os.path.exists(outf + ".png"):
#             continue
#         exp.add_report(
#             ScatterPlotReport(
#                 filter_algorithm=[
#                     alg1
#                     , alg2
#                 ],
#                 get_category=lambda run1, run2: run1['domain'],
#                 attributes=[atr],
#                 format='png',
#             ),
#             outfile=outf,
#         )



# for atr in ['expansions']:
#     for alg1, alg2 in [('ss-lmcut','ss-lmcut-ubreuse')]:
#         outf = os.path.join(exp.eval_dir, '{}-{}-vs-{}'.format(atr, alg1, alg2))
#         if os.path.exists(outf + ".png"):
#             continue

#         exp.add_report(
#             ScatterPlotReport(
#                 filter_algorithm=[
#                     alg1
#                     , alg2
#                 ],
#                 get_category=lambda run1, run2: run1['domain'],
#                 attributes=[atr],
#                 format='png',
#             ),
#             outfile=os.path.join(exp.eval_dir, '{}-{}-vs-{}'.format(atr, alg1, alg2)),
#         )


# for atr in ["search_time", 'total_time', 'total_follower_searches', 'follower_time']:
#     for alg1, alg2 in [('baseline-sbd','ss-sbd'), ('baseline-lmcut','ss-lmcut'),  ('ss-sbd','ss-sbd-cbff-1s'), ('ss-sbd-ubreuse-cbffpr-10s','ss-sbd-ubreuse-cbff-1s'), ('ss-sbd-ubreuse-cbff-10s','ss-sbd-ubreuse-cbffpr-10s'),]: # ('baseline-sbd','ss-sbd-up'),
#         outf = os.path.join(exp.eval_dir, '{}-{}-vs-{}'.format(atr, alg1, alg2))
#         if os.path.exists(outf + ".png"):
#             continue

#         exp.add_report(
#             ScatterPlotReport(
#                 filter_algorithm=[
#                     alg1
#                     , alg2
#                 ],
#                 get_category=lambda run1, run2: run1['domain'],
#                 attributes=[atr],
#                 format='png',
#             ),
#             outfile=os.path.join(exp.eval_dir, '{}-{}-vs-{}'.format(atr, alg1, alg2)),
#         )


for atr in ['total_time', 'optimal_solver_searches']:
    for alg1, alg2 in [('ss-sbd-ubreuse-cbff-1s','ss-sbd-ubreuse-cbffpr-1s'), ('ss-sbd-ubreuse-cbff-10s','ss-sbd-ubreuse-cbffpr-10s'), ('ss-sbd-ubreuse-cbffpr-1s','ss-sbd-ubreuse-cbffpr-5s'), ('ss-sbd-ubreuse-cbffpr-1s','ss-sbd-ubreuse-cbffpr-10s')]:
        outf = os.path.join(exp.eval_dir, '{}-{}-vs-{}'.format(atr, alg1, alg2))
        if os.path.exists(outf + ".png"):
            continue

        exp.add_report(
            ScatterPlotReport(
                filter_algorithm=[
                    alg1
                    , alg2
                ],
                get_category=lambda run1, run2: run1['domain'],
                attributes=[atr],
                format='png',
            ),
            outfile=os.path.join(exp.eval_dir, '{}-{}-vs-{}'.format(atr, alg1, alg2)),
        )


exp.run_steps()
