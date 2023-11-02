from task import Task, Operator, Mutex


def project_dict(dictionary, var_translation):
    return dict([(var_translation[var], val)
                 for var, val in dictionary.items()
                    if var_translation[var] is not None])


def project_operator(op, resource, var_translation, cost):
    name = op.name
    prevail = project_dict(op.prevail, var_translation)
    preposts = project_dict(op.preposts, var_translation)
    return Operator(name=name, prevail=prevail, preposts=preposts, cost=cost)

def project_mutex(mutex, var_translation):
    return Mutex(project_dict(mutex.facts, var_translation))

def filter_operators(operators):
    ops_by_changes = dict()
    for op in operators:
        if not op.preposts:
            # noop
            continue
        prev = frozenset(op.prevail.items())
        preposts = frozenset(op.preposts.items())
        changes = (prev, preposts)
        if changes not in ops_by_changes or ops_by_changes[changes].cost > op.cost:
            ops_by_changes[changes] = op
    return [op for c, op in sorted(ops_by_changes.items())]

def project(task, resource, op_costs):
    var_translation = range(len(task.variables))
    var_translation[resource.var_id] = None
    for i in xrange(resource.var_id + 1, len(task.variables)):
        var_translation[i] -= 1

    # Version
    version = task.version

    # Metric (1 = use operator costs)
    metric = 1

    # Variables
    variables = [v for i, v in enumerate(task.variables) if i != resource.var_id]

    # Mutexes
    mutexes = [project_mutex(mutex, var_translation) for mutex in task.mutexes]

    # Initial state
    initial_state = list(task.initial_state)
    del initial_state[resource.var_id]

    # Goals
    assert resource.var_id not in task.goals
    goals = project_dict(task.goals, var_translation)

    # Operators
    operators = [project_operator(op, resource, var_translation, cost)
                     for op, cost in zip(task.operators, op_costs)]
    operators = filter_operators(operators)

    return Task(version=version, metric=metric, variables=variables,
                mutexes=mutexes, initial_state=initial_state,
                goals=goals, operators=operators)
