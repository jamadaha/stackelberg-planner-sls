import re
import sys
import random
import argparse
import os
from shutil import copyfile


domain_location_regex_dic = {"logistics-strips": "city\d+-\d+"}


def parse_domain_specific_connections(domain_name, locations, connections, content):
    if domain_name == "logistics-strips":
        for i, loc1 in enumerate(locations):
            for loc2 in locations[i + 1:]:
                city1 = loc1[0:loc1.find('-')]
                city2 = loc2[0:loc2.find('-')]
                if city1 == city2:
                    connections.append((loc1, loc2))


def modify_problem_file(problem_file_name, new_problem_file_name):
    problem_file = open(problem_file_name, "r")
    content = problem_file.read()
    problem_file.close()

    # Assuming here that there is exactly this line in the domain file:
    # (:domain domain_name)
    domain_name = re.search(":domain .*\)", content).group()[8:-1]
    print domain_name

    objects = re.search(objects_regex, content).group()
    print objects

    locations = re.findall(domain_location_regex_dic[domain_name], objects)
    locations.sort()
    print locations

    # connections is a list of tuples of locations, where a tuple (a,b) represents that there is a road between a and b in both directions.
    # The locations are sorted, s.t. there is never tuple (b,a) iff a < b
    connections = []
    parse_domain_specific_connections(domain_name, locations, connections, content)
    print connections

    number_of_connections = (len(connections) * con_percent) / 100

    connections_subset = [connections[i] for i in sorted(random.sample(xrange(len(connections)), number_of_connections))]
    print connections_subset

    predicate = "(allowed_to_remove {0} {1})"
    predicates_to_insert = ""
    for con in connections_subset:
        predicates_to_insert += "\n" + predicate.format(con[0], con[1])
    print predicates_to_insert

    i = content.find("))")
    new_content = content[0:i + 1] + predicates_to_insert + content[i + 1:]
    new_problem_file = open(new_problem_file_name, "w")
    new_problem_file.write(new_content)
    new_problem_file.close()


p = argparse.ArgumentParser(description="")
p.add_argument("--dir", type=str, help="The director in which the problem files should be modified", default=None)
p.add_argument("--seed", type=int, help="Seed for random generator", default=42)
p.add_argument("--con-percent", type=int, help="Percentage of connections which should be included", default=100)
args = p.parse_args(sys.argv[1:])

dir = str(args.dir)
files_in_dir = os.listdir(dir)
print files_in_dir

random_seed = int(args.seed)
random.seed(random_seed)

con_percent = int(args.con_percent)

objects_regex = "\(:objects [^)]*\)"

print dir
new_dir = dir + "-rs" + str(random_seed) + "-pc" + str(con_percent)
os.makedirs(new_dir)

for file in files_in_dir:
    if file.find("domain") == -1:
        modify_problem_file(os.path.join(dir, file), os.path.join(new_dir, file))
    else:
        copyfile(os.path.join(dir, file), os.path.join(new_dir, file))









