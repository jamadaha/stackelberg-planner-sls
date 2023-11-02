NOTE: This is a fork of https://gitlab.com/atorralba_planners/stackelberg-planner-sls

# Stackelberg Framework and Symbolic Leader Search

This repository is an extension of FastDownward (see http://www.fast-downward.org/) that
implements the Stackelberg Planning framework detailed in these publications:

* Faster Stackelberg Planning via Symbolic Search and Information Sharing,
A. Torralba, P. Speicher, R. Kuennemann, M. Steinmetz and J. Hoffmann,
Proceedings of the 35th AAAI Conference on Artificial Intelligence (AAAI'21), Virtual Conference, 2021.

* Stackelberg Planning: Towards Effective Leader-Follower State Space Search,
P. Speicher, M. Steinmetz, M. Backes, J. Hoffmann, and R. Kuennemann,
Proceedings of the 32nd AAAI Conference on Artificial Intelligence (AAAI'18), New Orleans, USA, 2018.

Installation
==================

To build the tool, navigate to the /src folder and execute:
./build_all

Usage
==================

./fast-downward.py <instance> --search <configuration>

The recommended "default" configurations are:
* For easy instances: "sym_stackelberg(optimal_engine=symbolic(plan_reuse_minimal_task_upper_bound=false, plan_reuse_upper_bound=true), upper_bound_pruning=false)"
* For harder instances: "sym_stackelberg(optimal_engine=symbolic(plan_reuse_minimal_task_upper_bound=true, plan_reuse_upper_bound=true, force_bw_search_minimum_task_seconds=30, time_limit_seconds_minimum_task=300), upper_bound_pruning=true)"

The difference is weather you activate upper bound pruning, which requires some pre-processing. You may control the amount of pre-processing with the time limits: force_bw_search_minimum_task_seconds and time_limit_seconds_minimum_task


For the net-benefit planning variant use

./fast-downward.py <instance> --translate-options --soft 10000 --search-options --search <configuration>

This will set the reward for each individual goal to 10000 units  of cost.


PDDL Format
==================

The set of actions has to be divided in leader and follower actions. To specify this in PDDL we simply adopt the following convention:
 * Leader actions have a name that starts with fix_
 * Follower actions have a name that starts with attack_

 Note: this naming convention comes from a pentesting context where leader actions fix
 vulnerabilities in a network and the follower "attacks" the network by exploiting the
 remaining vulnerabilities.


Benchmarks
==================

The benchmarks folder contains the benchmarks used to evaluate the algorithms. We used the
following nomenclature:
 * rs42: a random seed of 42 was used to select which subset of actions is available for the leader.
 * tcX: where X is a number that specifies how many of the follower's actions the leader can disable.
 * -driving-: Benchmarks containing the word driving, the leader needs to move along the
   network in order to disable follower's actions.


Experiments
==================

The experiments/aaai21 folder contains the scripts used to run the experiments:
  * lab_parser.py: parses the output of the planner.
  * configs: all the configurations used for the experiments
  * create_configs.py: creates lab scripts, one for each config and puts them into a folder
  * run_scripts.sh: executes all lab scripts within the folder created by create_configs.py
  * report.py: fetches all results from all runs of all configs and re-writes some attributes for the scripts, the resulting properties file should be provided to paper-tables.py and paper-tables-soft-goals.py
  * paper-tables.py: Used to generate some of the plots in the paper, the resulting properties file should be provided to coverage-report.py
  * coverage-report.py: prints the coverage table included in the paper
  * paper-tables-soft-goals.py: Used to generate the plots in the paper that compare New vs Net benchmarks

The properties file provided is the one that was gathered by report.py, before being processed by paper-tables.py
