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
============

To build the tool, navigate to the /src folder and execute:
./build_all

Usage
======================

./fast-downward.py <instance> --search <configuration>

The recommended "default" configurations are:
* For easy instances: "sym_stackelberg(optimal_engine=symbolic(plan_reuse_minimal_task_upper_bound=false, plan_reuse_upper_bound=true), upper_bound_pruning=false)"
* For harder instances: "sym_stackelberg(optimal_engine=symbolic(plan_reuse_minimal_task_upper_bound=true, plan_reuse_upper_bound=true, force_bw_search_minimum_task_seconds=30, time_limit_seconds_minimum_task=300), upper_bound_pruning=true)"

The difference is weather you activate upper bound pruning, which requires some pre-processing. You may control the amount of pre-processing with the time limits: force_bw_search_minimum_task_seconds and time_limit_seconds_minimum_task


For the net-benefit planning variant use

./fast-downward.py <instance> --translate-options --soft 10000 --search-options --search <configuration>

This will set the reward for each individual goal to 10000 units  of cost.
