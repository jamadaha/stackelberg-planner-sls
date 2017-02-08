import cplex
from cplex.exceptions import CplexError

from collections import defaultdict


def find_operator_equivalence_classes(task, var_id):
    ops_by_changes = defaultdict(list)
    for op_id, op in enumerate(task.operators):
        prev = frozenset([(k, v) for k,v in op.prevail.items() if k != var_id])
        preposts = frozenset([(k, v) for k,v in op.preposts.items() if k != var_id])
        ops_by_changes[(prev, preposts)].append(op_id)
    return ops_by_changes.values()

def find_operator_costs(task, resource):
    try:
        c = cplex.Cplex()
        c.parameters.threads.set(1)
        c.objective.set_sense(c.objective.sense.maximize)

        operator_equivalence_classes = find_operator_equivalence_classes(task, resource.var_id)

        c.variables.add(names=["v%s" % i for i in xrange(len(resource.nodes))])
        c.variables.add(names=["vG"])
        c.variables.add(names=["o%s" % i for i in xrange(len(operator_equivalence_classes))],
                        obj=[len(ops) for ops in operator_equivalence_classes])

        init = resource.initial_value
        c.linear_constraints.add(lin_expr=[[["v%s" % init], [1.0]]], senses="E", rhs=[0.0])
        c.linear_constraints.add(lin_expr=[[["vG"], [1.0]]], senses="E", rhs=[1.0])

        for sink in resource.sinks:
            c.linear_constraints.add(lin_expr=[[["vG", "v%s" % sink], [1.0, -1.0]]], senses="G", rhs=[0.0])

        for class_id, ops in enumerate(operator_equivalence_classes):
            class_var = "o%s" % class_id
            for op in ops:
                preposts = task.operators[op].preposts
                if resource.var_id not in preposts:
                    c.linear_constraints.add(lin_expr=[[[class_var], [1.0]]],
                                             senses="E", rhs=[0.0])
                    continue
                pre, post = preposts[resource.var_id]
                c.linear_constraints.add(lin_expr=[[["v%s" % pre, "v%s" % post, class_var],
                                                    [1.0,        -1.0,         1.0]]],
                                         senses="L", rhs=[0.0])

        c.solve()
    except CplexError as exc:
        print exc
        return None

    op_costs = [0] * len(task.operators)
    for class_id, ops in enumerate(operator_equivalence_classes):
        cost = c.solution.get_values("o%s" % class_id)
        for op in ops:
            op_costs[op] = int(1000 * cost)
    return op_costs, 1000
