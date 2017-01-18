import argparse
import xml.etree.ElementTree as ET
import json
import sys
from fractions import Fraction
import math

DIGITS = 2
DENOMINATOR = int(math.pow(10, DIGITS))


def round_float(f):
    return float(("%%.%df" % DIGITS) % f)

exploit_actions = []
CVEs = []
hosts = ["internet"]
uniq_ports = []


class PDDLDomain(object):
    def __init__(self, name, exploit_actions, goal, uniq_ports, fixes = [], comment = None, apply_once=True):
        self.name = name
        self.exploit_actions = exploit_actions
        self.comment = comment
        self.goal = goal
        self.uniq_ports = uniq_ports
        self.fixes = fixes
        self.apply_once = apply_once
    def __str__(self):
        res = ''
        if self.comment != None:
            res += ';;; %s\n' % self.comment
        res += "(define (domain %s)\n" % self.name
        res += "; (:requirements strips)\n"
        res += "(:types host port vul"
        res += ")\n"
        res += "(:predicates\n"
        res += " " * 3
        for i in range(len(self.goal)):
            res += " (target%d_under_control)" % i
        res += "\n"
        res += " " * 4 + "(controlling ?h - host)\n"
        res += " " * 4 + "(connected ?src ?dest - host ?p - port)\n"
        res += " " * 4 + "(vuln_fixed ?h - host ?v - vul)\n"
        if self.apply_once:
            res += " " * 4 + "(applied ?h - host ?v - vul)\n"
        res += ")\n"
        res += "(:functions (total-cost))\n"
        res += "(:constants\n"
        res += " " * 4
        for i in range(0, len(hosts)):
            res += "%s " % hosts[i]
        res += "- host\n"
        res += " " * 4
        for p in self.uniq_ports:
            res += "p%d " % p
        res += "pnone - port\n"
        res += " " * 4
        uniq = []
        for vul in self.exploit_actions:
            if not vul in uniq:
                uniq.append(vul)
                res += "%s " % vul.CVE
        res += "- vul\n"
        res += ")\n"
        for i in range(len(self.goal)):
            for host in self.goal[i]:
                res += "(:action ATTACK_set_target%d_under_control_%s_1/1\n" % (i, host)
                res += " " * 4 + ":parameters ()\n"
                res += " " * 4 + ":precondition (and (not (target%d_under_control)) (controlling %s))\n" % (i, host)
                res += " " * 4 + ":effect (and (target%d_under_control) (increase (total-cost) 0))\n" % i
                res += ")\n"
        for vul in self.exploit_actions:
            f = Fraction(round_float(vul.prob)).limit_denominator(DENOMINATOR)
            res += "(:action ATTACK_exploit_%s_%s_%s\n" % (vul.CVE, vul.host, f)
            res += " " * 4 + ":parameters (?src - host)\n"
            res += " " * 4 + ":precondition (and (controlling ?src)"
            res += " (connected ?src %s p%d)" % (vul.host, vul.port)
            res += " (not (vuln_fixed %s %s))" % (vul.host, vul.CVE)

            if self.apply_once:
                res += " (not (applied %s %s))" % (vul.host, vul.CVE)
            res += ")\n"
            res += " " * 4 + ":effect (and (increase (total-cost) %d) (controlling %s)" % (vul.cost, vul.host)
            if self.apply_once:
                res += " (applied %s %s)" % (vul.host, vul.CVE)
            res += ")\n"
            res += ")\n"
        fix_action_scheme = 0
        for vul in self.exploit_actions:
            initial_cost = 0
            cost = 1
            res += "(:action FIX_exploit_%s_%s_%d#%d\n" % (vul.CVE, vul.host, initial_cost, fix_action_scheme)
            res += " " * 4 + ":parameters (?t - host)\n"
            res += " " * 4 + ":precondition (and (not (vuln_fixed ?t %s))" % vul.CVE
            res += ")\n"
            res += " " * 4 + ":effect (and (increase (total-cost) %d) (vuln_fixed ?t %s)" % (cost, vul.CVE)
            res += ")\n"
            res += ")\n"
            fix_action_scheme += 1
        res += ")\n"
        return res

class PDDLProblem(object):
    def __init__(self, name, domain_name, controlled, goal, comment = None):
        self.name = name
        self.domain_name = domain_name
        self.controlled = controlled
        self.goal = goal
        self.comment = comment
    def __str__(self):
        res = ''
        if self.comment != None:
            res += ";;; %s\n" % self.comment
        res += "(define (problem %s)\n"  % self.name
        res += "(:domain %s)\n" % self.domain_name
        res += "(:init\n"
        res += " " * 4 + "(= (total-cost) 0)\n"
        res += " " * 4 + "(controlling %s)\n" % self.controlled
        with open(args.net) as network_topology_file:
            res += network_topology_file.read()
        res += ")\n"
        res += "(:goal (and\n"
        for i in range(len(self.goal)):
            res += " " * 4 + "(target%d_under_control)\n" % i
        res += "))\n"
        res += "(:metric minimize (total-cost))\n"
        res += ")\n"
        return res


class ExploitAction(object):
    host = ""
    severity = 1
    port = 0
    risk_factor = "None"
    CVE = ""
    prob = 0
    cost = 0

    # The class "constructor" - It's actually an initializer
    def __init__(self, host, severity, port, risk_factor, CVE, prob, cost):
        self.host = host
        self.severity = severity
        self.port = port
        self.risk_factor = risk_factor
        self.CVE = CVE
        self.prob = prob
        self.cost = cost

    def __str__(self):
        return json.dumps(self.__dict__)


p = argparse.ArgumentParser(description="Nessus CoreSec Problem Generator")
p.add_argument("--nessus", type=str, help="the Nessus output file in xml format", default=None)
p.add_argument("--nvd", type=str, help="NVD vulnerability DB in json format", default=None)
p.add_argument("--net", type=str, help="The network topology in PDDL format", default=None)
p.add_argument("--disable-apply-once", help="disable apply once constraint", action="store_true", default=False)
p.add_argument("--domain", type=str, help="path to file where the PDDL domain will be stored", default="domain.pddl")
p.add_argument("--problem", type=str, help="path to file where the PDDL problem will be stored", default="problem.pddl")
args = p.parse_args(sys.argv[1:])


with open(args.nvd) as nvd_file:
   nvd_dict = json.load(nvd_file)


tree = ET.parse(args.nessus)
root = tree.getroot()
for child in root:
    print(child.tag, child.attrib)
report = root[1]
for reportHost in report:
    print(reportHost.tag, reportHost.attrib)
    name = reportHost.attrib['name']
    hosts.append(name)
    for reportItem in reportHost:
        print(reportItem.tag, reportItem.attrib)
        if (reportItem.tag == 'ReportItem'):
            protocol = reportItem.attrib['protocol']
            severity = int(reportItem.attrib['severity'])
            port = int(reportItem.attrib['port'])
            ecploit_CVEs = []
            for reportItemChild in reportItem:
                print(reportItemChild.tag, reportItemChild.attrib)
                if (reportItemChild.tag == 'risk_factor'):
                    risk_factor = reportItemChild.text
                if (reportItemChild.tag == 'cve'):
                    ecploit_CVEs.append(reportItemChild.text)
            if severity > 0:
                for cve in ecploit_CVEs:
                    vuln = ExploitAction(name, severity, port, risk_factor, cve, 0.5, 1)
                    exploit_actions.append(vuln)
                    if not cve in CVEs:
                        CVEs.append(cve)
                    if not port in uniq_ports:
                        uniq_ports.append(port)
print '[' + ', '.join(map(str, exploit_actions)) + ']'

for i in range(0, len(exploit_actions)):
    vuln = exploit_actions[i]

fixes = []
goal = [[hosts[len(hosts) - 1]]]

with open(args.domain, "w") as f:
    f.write(str(PDDLDomain("coresec", exploit_actions, goal, uniq_ports, fixes,
                               "Bla",
                               apply_once=not args.disable_apply_once)))

with open(args.problem, "w") as f:
    f.write(str(PDDLProblem("coresec", "coresec", hosts[0], goal,
                                "Bla")))



