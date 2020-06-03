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
    '8afa96ef3ce86a6b39ea031ac396f7b732dc43a5',
    'f503350c800543392b4054ca91b074252776dd34',
    '0ab0299b306776810922f24fe1c65df7bf82d3f8'
]

def rename_algorithm_and_domain(run):
    algo = run['config']
    dom = run['domain']
    for rev in REVISIONS:
        algo = algo.replace('{}-'.format(rev), '')
        algo = algo.replace('{}'.format(rev), '')

    dom = dom.replace("-robustness", "")

    if "-soft" in algo:
        algo = algo.replace("-soft", "")
        dom = dom + "-soft"

    run['algorithm'] = algo
    run['config'] = algo
    run['domain'] = dom
    return run

exp.add_fetcher('data/sbd',filter=[rename_algorithm_and_domain])

# ms_algorithm_time = Attribute('ms_algorithm_time', absolute=False, min_wins=True, functions=[geometric_mean])
# ms_atomic_algorithm_time = Attribute('ms_atomic_algorithm_time', absolute=False, min_wins=True, functions=[geometric_mean])
# ms_memory_delta = Attribute('ms_memory_delta', absolute=False, min_wins=True)
# fts_transformation_time = Attribute('fts_transformation_time', absolute=False, min_wins=True, functions=[geometric_mean])
# transformed_task_variables = Attribute('transformed_task_variables', absolute=False, min_wins=True, functions=[sum])
# transformed_task_labels = Attribute('transformed_task_labels', absolute=False, min_wins=True, functions=[sum])
# transformed_task_facts = Attribute('transformed_task_facts', absolute=False, min_wins=True, functions=[sum])
# transformed_task_transitions = Attribute('transformed_task_transitions', absolute=False, min_wins=True, functions=[sum])
# fts_search_task_construction_time = Attribute('fts_search_task_construction_time', absolute=False, min_wins=True, functions=[geometric_mean])
# search_task_variables = Attribute('search_task_variables', absolute=False, min_wins=True, functions=[sum])
# search_task_labels = Attribute('search_task_labels', absolute=False, min_wins=True, functions=[sum])
# search_task_facts = Attribute('search_task_facts', absolute=False, min_wins=True, functions=[sum])
# search_task_transitions = Attribute('search_task_transitions', absolute=False, min_wins=True, functions=[sum])
# fts_plan_reconstruction_time = Attribute('fts_plan_reconstruction_time', absolute=False, min_wins=True, functions=[geometric_mean])
# atomic_task_constructed = Attribute('atomic_task_constructed', absolute=True, min_wins=False)
# solved_without_search = Attribute('solved_without_search', absolute=True, min_wins=True)
# extra_attributes = [
# ms_algorithm_time,
#     ms_atomic_algorithm_time,
#     ms_memory_delta,
#     fts_transformation_time,
#     transformed_task_variables,
#     transformed_task_labels,
#     transformed_task_facts,
#     transformed_task_transitions,
#     fts_search_task_construction_time,
#     search_task_variables,
#     search_task_labels,
#     search_task_facts,
#     search_task_transitions,
#     fts_plan_reconstruction_time,
#     atomic_task_constructed,
#     solved_without_search,
# ]


attributes = ['search_time', 'memory', 'total_time', 'error', 'coverage', 'pareto_frontier_size', 'follower_time', 'leader_time', 'optimally_solved_subproblems', 'total_follower_searches', 'optimal_solver_searches']
# attributes.extend(extra_attributes)


## HTML reports

exp.add_report(AbsoluteReport(attributes=attributes))

### Latex reports

## expansion plots for bisim


for atr in ['optimally_solved_subproblems', "search_time", 'total_time', 'total_follower_searches', 'follower_time']:
    for alg1, alg2 in [('ss-sbd','ss-sbd-ubreuse'), ('ss-sbd','ss-sbd-up'), ('ss-sbd', 'ss-sbd-up-ubreuse-tlim') ]: 
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
