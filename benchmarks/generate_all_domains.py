import re
import sys
import random
import argparse
import os
from shutil import copyfile

domains_array = ["logistics98-robustness", "logistics98-robustness-driving", "no-mystery-robustness-driving", "rovers-robustness-driving", "sokoban-opt11-strips-robustness-walls", "tpp-robustness-driving", "transport-opt11-strips-robustness-driving", "transport-opt14-strips-robustness-driving", "visitall-opt14-strips-robustness-driving"]
number_of_cons_array = ["0", "1", "2", "3", "4", "5", "6", "8", "10", "12", "16", "25", "32", "50", "64", "128", "256", "512", "1024", "2048", "4096", "8192"]

for domain in domains_array:
    for i, cons in enumerate(number_of_cons_array):
        if i== 0:
            continue
        #print "python modify-problems.py --dir " + domain + " --con-total " + cons + " --at-least-con " + number_of_cons_array[i-1]
        os.system("python modify-problems.py --dir " + domain + " --con-total " + cons + " --at-least-con " + number_of_cons_array[i-1])