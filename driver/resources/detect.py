#!/usr/bin/env python

from task import parse_task, write_task
from dtg import DTG
from projection import project
from resourcecost import find_operator_costs


MIN_AVAILABILITY = 5
MAX_REL_NUM_OPS_IN_PROJECTION = 0.85


def detect_resources(task):
    resources = []
    for var_id, var in enumerate(task.variables):
        if var_id not in task.goals:
            dtg = DTG(task, var_id)
            if dtg.is_resource:
                resources.append(dtg)
    return resources


def project_out_largest_resource(input_filename, output_filename):
    task = parse_task(input_filename)
    resources = detect_resources(task)
    best = None
    for resource in resources:
        if best is None or resource.resource_availability > best.resource_availability:
            best = resource
    if best is None or best.resource_availability < MIN_AVAILABILITY:
        return None

    print "Largest resource: variable %d (%s) with availability %d" % (
        best.var_id, task.variables[best.var_id].facts[0], best.resource_availability)

    op_costs, limit = find_operator_costs(task, best)
    projected_task = project(task, best, op_costs)
    rel_num_ops_in_projection = len(projected_task.operators) / float(len(task.operators))
    print "Relative number of operators: %d/%d = %f" % (
        len(projected_task.operators),
        len(task.operators),
        rel_num_ops_in_projection)
    num_ops_with_costs = len([op for op in projected_task.operators if op.cost != 0])
    percentage_of_ops_with_costs = num_ops_with_costs / float(len(projected_task.operators))
    print "Percentage of operators with costs: %d/%d = %f" % (
        num_ops_with_costs, len(projected_task.operators), percentage_of_ops_with_costs)
    if rel_num_ops_in_projection > MAX_REL_NUM_OPS_IN_PROJECTION:
        return None
    write_task(projected_task, output_filename)
    print "Resource projected task written to '%s'" % output_filename
    return limit


if __name__ == "__main__":
    limit = project_out_largest_resource("output", "output.resource")
    print "Now run a search with f-bound=%s" % limit
