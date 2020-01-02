#! /usr/bin/env python

from lab.parser import Parser
from collections import defaultdict
import re

eval = Parser()

def set_coverage(content, props):
    if props['search_returncode'] == 1:
        props['coverage'] = 1
    else:
        props['coverage'] = 0


eval.add_function(set_coverage)

eval.parse() 
