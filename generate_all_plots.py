import re
import sys
import random
import argparse
import os
from shutil import copyfile

only_opt_configs = [#'IDS with DEADPDB, LM-cut, w/o AUBP',
                      'IDS with DEADPDB, LM-cut',
                      #'IDS with DEADPDB, LM-cut, w/o PAPA',
                      'IDS with DEADPDB, LM-cut, w/o (real)PAPA',
                      'IDS with DEADPDB, LM-cut, w/o POR',
                      #'IDS with DEADPDB, LM-cut, w/o POR PAPA AUBP',
                      #'DFS with DEADPDB, LM-cut, w/o POR PAPA AUBP',
                      'DFS with DEADPDB, LM-cut, w/o POR (real)PAPA AUBP',
                      'DFS with DEADPDB, LM-cut, w/o AUBP']
only_opt_configs_str = ""
for str in only_opt_configs:
    only_opt_configs_str += " \'" + str + "'"

only_sat_configs = [#'IDS with DEADPDB, FF, w/o POR PAPA AUBP',
                      #'IDS with DEADPDB, FF, w/o AUBP',
                      #'IDS with DEADPDB, FF, w/o PAPA',
                      'IDS with DEADPDB, FF, w/o (real)PAPA',
                      'IDS with DEADPDB, FF',
                      'IDS with DEADPDB, FF, and sorting fix ops, w/o POR',
                      #'DFS with DEADPDB, FF, w/o POR PAPA AUBP',
                      'DFS with DEADPDB, FF, w/o POR (real)PAPA AUBP',
                      'DFS with DEADPDB, FF, w/o AUBP']
only_sat_configs_str = ""
for str in only_sat_configs:
    only_sat_configs_str += " \'" + str + "\'"

pentesting_dirs = '~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-04-09-17-2/ ~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-05-09-17/ ~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-06-09-17/ ~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-08-09-17/'
robustness_wo_driving_dirs = '~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-01-09-17/ ~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-05-09-17/ ~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-06-09-17/'
robustness_driving_dirs = '~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-31-08-17/ ~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-04-09-17/ ~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-05-09-17/ ~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-06-09-17/'
all_robustness_dirs = '~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-31-08-17/ ~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-04-09-17/ ~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-01-09-17/ ~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-05-09-17/ ~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-06-09-17/ ~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-08-09-17/'
transport_dirs = '~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-05-09-17-2/'
rovers_dirs = '~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-22-09-17/'
no_mystery_robustness_driving_fuel_dirs = '~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-27-09-17/'

pentesting_domains = 'pentesting-robustness-rs42'
robustness_wo_driving_domains = 'logistics98-robustness-rs42 no-mystery-robustness-rs42 sokoban-opt11-strips-robustness-rs42 visitall-opt14-strips-robustness-rs42'
robustness_driving_domains = 'logistics98-robustness-driving-rs42 no-mystery-robustness-driving-rs42 sokoban-opt11-strips-robustness-walls-rs42 visitall-opt14-strips-robustness-driving-rs42'
all_robustness_domains = 'logistics98-robustness-rs42 no-mystery-robustness-rs42 sokoban-opt11-strips-robustness-rs42 visitall-opt14-strips-robustness-rs42'


print 1
os.system("python parse_properties_and_generate_plots.py --dir " + pentesting_dirs + " --configs" + only_opt_configs_str + " --domains " + pentesting_domains + " --name-suffix _opt")
print 2
os.system("python parse_properties_and_generate_plots.py --dir " + all_robustness_dirs + " --configs" + only_opt_configs_str + " --domains " + all_robustness_domains + " --name-suffix _opt")


print 3

os.system("python parse_properties_and_generate_plots.py --dir " + pentesting_dirs + " --configs" + only_sat_configs_str + " --domains " + pentesting_domains + " --name-suffix _sat")
print 4
os.system("python parse_properties_and_generate_plots.py --dir " + all_robustness_dirs + " --configs" + only_sat_configs_str + " --domains " + all_robustness_domains + " --name-suffix _sat")


print 5
os.system("python parse_properties_and_generate_plots.py --dir " + rovers_dirs + " --configs" + only_sat_configs_str + " --name-suffix _sat")
os.system("python parse_properties_and_generate_plots.py --dir " + rovers_dirs + " --configs" + only_opt_configs_str + " --name-suffix _opt")
print 6

os.system("python parse_properties_and_generate_plots.py --dir " + no_mystery_robustness_driving_fuel_dirs + " --configs" + only_sat_configs_str + " --name-suffix _sat")
os.system("python parse_properties_and_generate_plots.py --dir " + no_mystery_robustness_driving_fuel_dirs + " --configs" + only_opt_configs_str + " --name-suffix _opt")

print 7

os.system("python generate_pareto_plots.py --dir ~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-31-08-17 --domain no-mystery-robustness-driving-rs42 --config 'IDS with DEADPDB, LM-cut'")
print 8
os.system("python generate_pareto_plots.py --dir ~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-31-08-17 --domain visitall-opt14-strips-robustness-driving-rs42 --config 'IDS with DEADPDB, LM-cut' --tcs 5 10 16 50")
print 9





#os.system("python parse_properties_and_generate_plots.py --dir " + robustness_wo_driving_dirs + " --configs" + only_opt_configs_str + " --domains " + robustness_wo_driving_domains + " --name-suffix _opt")
#os.system("python parse_properties_and_generate_plots.py --dir " + robustness_driving_dirs + " --configs" + only_opt_configs_str + " --domains " + robustness_driving_domains + " --name-suffix _opt")
#os.system("python parse_properties_and_generate_plots.py --dir " + robustness_wo_driving_dirs + " --configs" + only_sat_configs_str + " --domains " + robustness_wo_driving_domains + " --name-suffix _sat")
#os.system("python parse_properties_and_generate_plots.py --dir " + robustness_driving_dirs + " --configs" + only_sat_configs_str + " --domains " + robustness_driving_domains + " --name-suffix _sat")

#os.system("python parse_properties_and_generate_plots.py --dir " + transport_dirs + " --configs" + only_sat_configs_str + " --name-suffix _sat")
#os.system("python parse_properties_and_generate_plots.py --dir " + transport_dirs + " --configs" + only_opt_configs_str + " --name-suffix _opt")