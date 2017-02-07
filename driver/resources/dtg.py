class DTGNode(object):
    def __init__(self, value):
        self.original_value = value
        self.topological_order = None
        self.successors = set()
        self.visited = False
        self.part_of_current_branch = False

class DTG(object):
    def __init__(self, task, var_id):
        self.var_id = var_id
        facts = task.variables[var_id].facts
        self.nodes = [DTGNode(val) for val in xrange(len(facts))]
        for op in task.operators:
            if var_id in op.preposts:
                pre, post = op.preposts[var_id]
                if pre == -1:
                    self.is_resource = False
                    return
                self.nodes[pre].successors.add(self.nodes[post])
        self.initial_value = task.initial_state[var_id]
        acyclic, next_id  = _dfs(self.nodes[self.initial_value], 0)
        self.is_resource = acyclic
        self.resource_availability = next_id - 1
        self.sinks = [i for i, n in enumerate(self.nodes) if not n.successors]

    def get_operator_cost(self, op):
        assert self.is_resource
        if self.var_id in op.preposts:
            pre, post = op.preposts[self.var_id]
            return self.nodes[pre].topological_order - self.nodes[post].topological_order
        else:
            return 0

    def dump(self, filename, task):
        with open(filename, "w") as f:
            f.write("digraph g {\n")
            for node in self.nodes:
                f.write("""%s [label="%s (%s)"];""" % (
                    node.original_value,
                    task.variables[self.var_id].facts[node.original_value],
                    node.topological_order))
                for s in node.successors:
                    f.write("    %s -> %s;\n" % (node.original_value, s.original_value))
            f.write("}")


def _dfs(node, next_id):
    if node.part_of_current_branch:
        # Not a DAG
        return False, 0
    if not node.visited:
        node.part_of_current_branch = True
        for succ in node.successors:
            acyclic, next_id = _dfs(succ, next_id)
            if not acyclic:
                return False, 0
        node.part_of_current_branch = False
        node.visited = True
        node.topological_order = next_id
        next_id += 1
    return True, next_id
