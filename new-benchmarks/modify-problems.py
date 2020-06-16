import re
import sys
import random
import argparse
import os
from shutil import copyfile


domain_location_regex_dic = {"logistics-strips": "city\d+-\d+",
                             "Rover": "waypoint\d+",
                             "TPP-Propositional": "market\d+|depot\d+",
                             "transport": "city-?\d*-loc-\d+",
                             "grid-visit-all": "loc-x\d+-y\d+",
                             "sokoban-sequential": "pos-\d+-\d+",
                             "pipesworld_strips": "B\d+"}
connection_predicate_regex = {"no-mystery-strips": "connected",
                              "TPP-Propositional": "connected",
                              "transport": "road",
                              "grid-visit-all": "connected"}

makes_sense_to_increase_number_of_connections = False

def parse_domain_specific_locations(domain_name, locations, objects, content):
    if domain_name == "logistics" or domain_name == "Rover" or domain_name == "TPP-Propositional" or domain_name == "transport" or domain_name == "grid-visit-all" or domain_name == "sokoban-sequential" or domain_name == "pipesworld_strips":
        for x in re.findall(domain_location_regex_dic[domain_name], objects):
            locations.append(x)
    elif domain_name == "no-mystery-strips":
        for loc in objects.split():
            loc.replace(")", "")
            if content.find("(location " + loc + ")") != -1 :
                locations.append(loc)


def parse_domain_specific_connections(domain_name, locations, connections, content):
    if domain_name == "logistics":
        for i, loc1 in enumerate(locations):
            for loc2 in locations[i + 1:]:
                city1 = loc1[0:loc1.find('-')]
                city2 = loc2[0:loc2.find('-')]
                if city1 == city2:
                    connections.append((loc1, loc2))
    elif domain_name == "no-mystery-strips" or domain_name == "TPP-Propositional" or domain_name == "transport" or domain_name == "grid-visit-all":
        for i, loc1 in enumerate(locations):
            for loc2 in locations[i + 1:]:
                if content.find("(" + connection_predicate_regex[domain_name] + " " + loc1 + " " + loc2 + ")") != -1 :
                    if content.find("(" + connection_predicate_regex[domain_name] + " " + loc2 + " " + loc1 + ")") != -1:
                        connections.append((loc1, loc2))
                    else:
                        print("We assumed here that all roads are bi-directional which seems not to be the case for " + loc1 + " and " + loc2 + "... Abort!")
                        exit()
    elif domain_name == "Rover":
        for i, loc1 in enumerate(locations):
            for loc2 in locations[i + 1:]:
                if re.search("\(can_traverse rover\d+ " + loc1 + " " + loc2 + "\)", content) is not None:
                    if re.search("\(can_traverse rover\d+ " + loc2 + " " + loc1 + "\)", content) is not None:
                        connections.append((loc1, loc2))
                    else:
                        print("We assumed here that all roads in Rovers are bi-directional which seems not to be the case for " + loc1 + " and " + loc2 + "... Abort!")
                        exit()
    elif domain_name == "sokoban-sequential":
            for loc in locations:
                if content.find("(clear " + loc + ")") != -1 and re.search("\(at \w+-\d+ " + loc + "\)", content) is None:
                    connections.append((loc, loc))
    elif domain_name == "pipesworld_strips":
            for loc in locations:
                if re.search("\(on " + loc + " A\d+\)", content) is None:
                    connections.append((loc, loc))


def generate_walls_for_sokoban(locations, content):
    walls = []
    for loc in locations:
        if content.find("(clear " + loc + ")") == -1 and re.search("\(at \w+-\d+ " + loc + "\)", content) is None:
            walls.append(loc)
    predicate1 = "(block-loc {0})"
    predicate2 = "(MOVE-DIR {0} {1} {2})"
    predicates_to_insert = ""
    for loc1 in walls:
        predicates_to_insert += "\n" + predicate1.format(loc1)
        loc1_minus1 = loc1.find("-")
        loc1_minus2 = loc1.find("-", loc1_minus1 + 1)
        loc1_x = int(loc1[loc1_minus1+1:loc1_minus2])
        loc1_y = int(loc1[loc1_minus2+1:])

        for loc2 in locations:
            if loc1 == loc2 or loc2 in walls:
                continue
            loc2_minus1 = loc2.find("-")
            loc2_minus2 = loc2.find("-", loc2_minus1 + 1)
            loc2_x = int(loc2[loc2_minus1 + 1:loc2_minus2])
            loc2_y = int(loc2[loc2_minus2 + 1:])

            if loc1_x == loc2_x and (loc1_y+1) == loc2_y:
                predicates_to_insert += "\n" + predicate2.format(loc1, loc2, "dir-down")
            if loc1_x == loc2_x and (loc1_y-1) == loc2_y:
                predicates_to_insert += "\n" + predicate2.format(loc1, loc2, "dir-up")
            if (loc1_x+1) == loc2_x and loc1_y == loc2_y:
                predicates_to_insert += "\n" + predicate2.format(loc1, loc2, "dir-right")
            if (loc1_x-1) == loc2_x and loc1_y == loc2_y:
                predicates_to_insert += "\n" + predicate2.format(loc1, loc2, "dir-left")
    return predicates_to_insert


def generate_random_connection_sublist (connection_list, new_size):
    indizes = list(range(len(connection_list)))
    selected_indizes = []
    while len(selected_indizes < new_size):
        x = random.randint(0, len(indizes)-1)
        selected_indizes.append(indizes[x])
        del indizes[x]

    return [connection_list[i] for i in sorted(selected_indizes)]


def modify_problem_file(problem_file_name, new_problem_file_name):
    global makes_sense_to_increase_number_of_connections
    print(problem_file_name)

    problem_file = open(problem_file_name, "r")
    content = problem_file.read()
    problem_file.close()

    # Assuming here that there is exactly this line in the domain file:
    # (:domain domain_name)
    domain_name = re.search(":domain .*\)", content).group()[8:-1]
    print(domain_name)

    objects = re.search(objects_regex, content).group()
    print(objects)

    locations = []
    parse_domain_specific_locations(domain_name, locations, objects, content)
    locations.sort()
    print("locations:")
    print(locations)

    # connections is a list of tuples of locations, where a tuple (a,b) represents that there is a road between a and b in both directions.
    # The locations are sorted, s.t. there is never tuple (b,a) iff a < b
    connections = []
    parse_domain_specific_connections(domain_name, locations, connections, content)
    print(connections)

    if con_total is not None:
        number_of_connections = min(con_total, len(connections))
        if number_of_connections < len(connections):
            makes_sense_to_increase_number_of_connections = True
        else:
            if len(connections) <= at_least_con:
                return
    else:
        number_of_connections = (len(connections) * con_percent) / 100

    random.seed(random_seed)
    connections_subset = [connections[i] for i in sorted(random.sample(xrange(len(connections)), number_of_connections))]
    #connections_subset = generate_random_connection_sublist(connections, number_of_connections)
    print(connections_subset)

    predicate = "(allowed_to_remove {0} {1})"
    predicates_to_insert = ""
    for con in connections_subset:
        predicates_to_insert += "\n" + predicate.format(con[0], con[1])
    if domain_name == "sokoban-sequential":
        predicates_to_insert += generate_walls_for_sokoban(locations, content)
    print(predicates_to_insert)

    i = content.find("(:init")
    new_content = content[0:i + 6] + predicates_to_insert + content[i + 6:]
    new_problem_file = open(new_problem_file_name, "w")
    new_problem_file.write(new_content)
    new_problem_file.close()


p = argparse.ArgumentParser(description="")
p.add_argument("--dir", type=str, help="The director in which the problem files should be modified", default=None)
p.add_argument("--seed", type=int, help="Seed for random generator", default=42)
p.add_argument("--con-percent", type=int, help="Percentage of connections which should be included", default=100)
p.add_argument("--con-total", type=int, help="total number of connections which should be included", default=None)
p.add_argument("--at-least-con", type=int, help="Only generate new problems if there are actually more connections then given here ", default=0)
args = p.parse_args(sys.argv[1:])

dir = str(args.dir)
files_in_dir = os.listdir(dir)
print(files_in_dir)

random_seed = int(args.seed)

con_percent = int(args.con_percent)
con_total = None if args.con_total is None else int(args.con_total)
at_least_con = int(args.at_least_con)

objects_regex = "\(:objects[^)]*\)"

print(dir)
new_dir = dir + "-rs" + str(random_seed) + ("-tc" + str(con_total) if con_total is not None else "-pc" + str(con_percent))
if not os.path.exists(new_dir):
    os.makedirs(new_dir)

for file in files_in_dir:
    if file.find("domain") == -1:
        modify_problem_file(os.path.join(dir, file), os.path.join(new_dir, file))
    else:
        copyfile(os.path.join(dir, file), os.path.join(new_dir, file))

if makes_sense_to_increase_number_of_connections is False:
    print("Increasing total number of connections makes no sense for this domain!")
    exit(1)
else:
    exit(0)








