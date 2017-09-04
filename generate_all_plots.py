import re
import sys
import random
import argparse
import os
from shutil import copyfile

only_opt_configs = ['IDS with DEADPDB, LM-cut, w/o AUBP',
                      'IDS with DEADPDB, LM-cut',
                      'IDS with DEADPDB, LM-cut, w/o PAPA',
                      'IDS with DEADPDB, LM-cut, w/o POR',
                      'IDS with DEADPDB, LM-cut, w/o POR PAPA AUBP']
only_opt_configs_str = ""
for str in only_opt_configs:
    only_opt_configs_str += " \'" + str + "\'"

only_sat_configs = ['IDS with DEADPDB, FF, w/o POR PAPA AUBP',
                      'IDS with DEADPDB, FF, w/o AUBP',
                      'IDS with DEADPDB, FF, w/o PAPA',
                      'IDS with DEADPDB, FF',
                      'IDS with DEADPDB, FF, and sorting fix ops, w/o POR']
only_sat_configs_str = ""
for str in only_sat_configs:
    only_sat_configs_str += " \'" + str + "\'"

pentesting_dirs = '~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-03-09-17/'
robustness_wo_driving_dirs = '~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-01-09-17/'
robustness_driving_dirs = '~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-31-08-17/ ~/Documents/SimPentestWhatIfStuff/sim-pentest-whatif-04-09-17/'

print 1
os.system("python parse_properties_and_generate_plots.py --dir " + pentesting_dirs + " --configs" + only_opt_configs_str + " --name-suffix _opt")
print 2
os.system("python parse_properties_and_generate_plots.py --dir " + robustness_wo_driving_dirs + " --configs" + only_opt_configs_str + " --name-suffix _opt")
print 3
os.system("python parse_properties_and_generate_plots.py --dir " + robustness_driving_dirs + " --configs" + only_opt_configs_str + " --name-suffix _opt")
print 4

os.system("python parse_properties_and_generate_plots.py --dir " + pentesting_dirs + " --configs" + only_sat_configs_str + " --name-suffix _sat")
print 5
os.system("python parse_properties_and_generate_plots.py --dir " + robustness_wo_driving_dirs + " --configs" + only_sat_configs_str + " --name-suffix _sat")
print 6
os.system("python parse_properties_and_generate_plots.py --dir " + robustness_driving_dirs + " --configs" + only_sat_configs_str + " --name-suffix _sat")
print 7