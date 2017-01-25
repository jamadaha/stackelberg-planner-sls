import sys
from subprocess import call, Popen, PIPE

call(['python', 'parse_nessus_vulnerabilities.py'] + sys.argv[1:])
p = Popen(['./plan.py', 'problem.pddl',  '--heuristic','h1=attack_success_prob_reuse(default_heuristic=blind)', '--heuristic', 'h2=lmcut(cost_type=3)', '--heuristic', 'h3=budget_dead_end(budget_heuristic=h2, prob_cost_heuristic=h1)', '--search', 'fixsearch(search_engine=astar(h3), attack_heuristic=h3, initial_attack_budget=40, initial_fix_budget=100000)'], stdin=PIPE, stdout=PIPE, stderr=PIPE)
output, err = p.communicate()
index1 = output.find("Resulting Pareto-frontier:")
index2 = output.find("\nEND Pareto-frontier")
print output[index1:index2]
