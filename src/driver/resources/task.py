def read_line(infile):
    return next(infile).strip()


def read_number(infile):
    return int(read_line(infile))


def check_magic(infile, magic):
    assert read_line(infile) == magic


def read_partial_state(infile):
    values = {}
    for _ in xrange(read_number(infile)):
        var, val = read_line(infile).split()
        values[int(var)] = int(val)
    return values

def write_line(line, outfile):
    outfile.write("%s\n" % line)

def write_partial_state(values, outfile):
    write_line(len(values), outfile)
    for var, val in sorted(values.items()):
        write_line("%s %s" % (var, val), outfile)



class Operator(object):
    def __init__(self, name="", prevail=None, preposts=None, cost=0):
        self.name = name
        self.prevail = prevail or {}
        self.preposts = preposts or {}
        self.cost = cost

    def read(self, infile):
        check_magic(infile, "begin_operator")
        self.name = read_line(infile)
        self.prevail = read_partial_state(infile)
        self.preposts = {}
        for _ in xrange(read_number(infile)):
            effect_line = read_line(infile).split()
            assert len(effect_line) == 4, "Effect conditions not supported"
            n, var, pre, post = map(int, effect_line)
            self.preposts[var] = (pre, post)
        self.cost = read_number(infile)
        check_magic(infile, "end_operator")
        return self

    def write(self, outfile):
        write_line("begin_operator", outfile)
        write_line(self.name, outfile)
        write_partial_state(self.prevail, outfile)

        write_line(len(self.preposts), outfile)
        for var, (pre, post) in sorted(self.preposts.items()):
            write_line("0 %s %s %s" % (var, pre, post), outfile)

        write_line(self.cost, outfile)
        write_line("end_operator", outfile)

class Mutex(object):
    def __init__(self, facts=None):
        self.facts = facts or {}

    def read(self, infile):
        check_magic(infile, "begin_mutex_group")
        self.facts = read_partial_state(infile)
        check_magic(infile, "end_mutex_group")
        return self

    def write(self, outfile):
        write_line("begin_mutex_group", outfile)
        write_partial_state(self.facts, outfile)
        write_line("end_mutex_group", outfile)

class Variable(object):
    def __init__(self, infile):
        self.read(infile)

    def read(self, infile):
        check_magic(infile, "begin_variable")
        self.name = read_line(infile)
        self.axiom_layer = read_number(infile)
        self.facts = []
        for _ in xrange(read_number(infile)):
            self.facts.append(read_line(infile))
        check_magic(infile, "end_variable")

    def write(self, outfile):
        write_line("begin_variable", outfile)
        write_line(self.name, outfile)
        write_line(self.axiom_layer, outfile)
        write_line(len(self.facts), outfile)
        for fact in self.facts:
            write_line(fact, outfile)
        write_line("end_variable", outfile)


class Task(object):
    def __init__(self, version=None, metric=None, variables=None,
                 mutexes=None, initial_state=None, goals=None, operators=None):
        self.version = version
        self.metric = metric
        self.variables = variables or []
        self.mutexes = mutexes or []
        self.initial_state = initial_state or []
        self.goals = goals or {}
        self.operators = operators or []


    def read(self, infile):
        check_magic(infile, "begin_version")
        self.version = read_number(infile)
        check_magic(infile, "end_version")

        check_magic(infile, "begin_metric")
        self.metric = read_number(infile)
        check_magic(infile, "end_metric")

        self.variables = []
        for i in xrange(read_number(infile)):
            self.variables.append(Variable(infile))

        self.mutexes = []
        for _ in xrange(read_number(infile)):
            self.mutexes.append(Mutex().read(infile))

        self.initial_state = []
        check_magic(infile, "begin_state");
        for _ in xrange(len(self.variables)):
            self.initial_state.append(read_number(infile))
        check_magic(infile, "end_state");

        check_magic(infile, "begin_goal");
        self.goals = read_partial_state(infile)
        check_magic(infile, "end_goal");

        self.operators = []
        for i in xrange(read_number(infile)):
            self.operators.append(Operator().read(infile))

        assert read_number(infile) == 0, "Axioms not supported"
        # Rest of file is ignored by planner.
        return self


    def write(self, outfile):
        write_line("begin_version", outfile)
        write_line(self.version, outfile)
        write_line("end_version", outfile)

        write_line("begin_metric", outfile)
        write_line(self.metric, outfile)
        write_line("end_metric", outfile)

        write_line(len(self.variables), outfile)
        for var in self.variables:
            var.write(outfile)

        write_line(len(self.mutexes), outfile)
        for mutex in self.mutexes:
            mutex.write(outfile)

        write_line("begin_state", outfile)
        for val in self.initial_state:
            write_line(val, outfile)
        write_line("end_state", outfile)

        write_line("begin_goal", outfile)
        write_partial_state(self.goals, outfile)
        write_line("end_goal", outfile)

        write_line(len(self.operators), outfile)
        for op in self.operators:
            op.write(outfile)

        # Axioms
        write_line(0, outfile)
        # Stuff that we ignored earlier.
        # The planner ignores it too, but checks that it is there.
        write_line("begin_SG\nend_SG\nbegin_DTG", outfile)



def parse_task(filename):
    with open(filename) as infile:
        return Task().read(infile)

def write_task(task, filename):
    with open(filename, "w") as outfile:
        return task.write(outfile)
