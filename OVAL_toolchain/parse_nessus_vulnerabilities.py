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
open_ports_for_host = {'internet': []}
compromised_types = ['confidentiality', 'integrity', 'availability']
protocols = []


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
        res += "(:types host port vul compromised_type"
        res += ")\n"
        res += "(:predicates\n"
        res += " " * 3
        for i in range(len(self.goal)):
            res += " (target%d_under_control)" % i
        res += "\n"
        res += " " * 4 + "(compromised ?h - host ?ct - compromised_type)\n"
        res += " " * 4 + "(connected ?src ?dest - host ?po - port ?pr - protocol)\n"
        res += " " * 4 + "(vul_exists ?v - vul ?h - host ?po - port ?pr - protocol)\n"
        if self.apply_once:
            res += " " * 4 + "(applied ?v - vul ?h - host)\n"
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
        for cve in CVEs:
            res += "%s " % cve
        res += "- vul\n"
        res += " " * 4
        for ct in compromised_types:
            res += "%s " % ct
        res += "- compromised_type\n"
        res += " " * 4
        for protocol in protocols:
            res += "%s " % protocol
        res += "- protocol\n"
        res += ")\n"
        for i in range(len(self.goal)):
            for host in self.goal[i]:
                res += "(:action ATTACK_set_target%d_under_control_%s_1/1\n" % (i, host)
                res += " " * 4 + ":parameters ()\n"
                res += " " * 4 + ":precondition (and (not (target%d_under_control)) (compromised %s integrity))\n" % (i, host)
                res += " " * 4 + ":effect (and (target%d_under_control) (increase (total-cost) 0))\n" % i
                res += ")\n"
        for cve in CVEs:
            f = Fraction(round_float(get_prob_of_CVE(cve))).limit_denominator(DENOMINATOR)
            res += "(:action ATTACK_exploit_%s_%s\n" % (cve, f)
            res += " " * 4 + ":parameters (?src - host ?t - host ?po - port ?pr - protocol)\n"
            res += " " * 4 + ":precondition (and (compromised ?src integrity)"
            res += " (connected ?src ?t ?po ?pr)"
            res += " (vul_exists %s ?t ?po ?pr)" % cve

            if self.apply_once:
                res += " (not (applied %s ?t))" % cve
            res += ")\n"
            res += " " * 4 + ":effect (and (increase (total-cost) %d)" % get_cost_of_CVE(cve)
            for ct in get_compromised_types_of_CVE(cve):
                res += " (compromised ?t %s)" % ct
            if self.apply_once:
                res += " (applied %s ?t)" % cve
            res += ")\n"
            res += ")\n"
        fix_action_scheme = 0
        for cve in CVEs:
            initial_cost = 1
            cost = 1
            res += "(:action FIX_exploit_%s_%d#%d\n" % (cve, initial_cost, fix_action_scheme)
            res += " " * 4 + ":parameters (?t - host)\n"
            res += " " * 4 + ":precondition (and (vul_exists %s ?t)" % cve
            res += ")\n"
            res += " " * 4 + ":effect (and (increase (total-cost) %d) (not (vul_exists ?t %s))" % (cost, cve)
            res += ")\n"
            res += ")\n"
            fix_action_scheme += 1
        res += ")\n"
        return res

class PDDLProblem(object):
    def __init__(self, name, domain_name, exploit_actions, controlled, goal, comment = None):
        self.name = name
        self.domain_name = domain_name
        self.exploit_actions = exploit_actions
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
        if args.net != None:
            with open(args.net) as network_topology_file:
                res += network_topology_file.read()
        else:
            for host_source in hosts:
                for host_target in hosts:
                    if host_source != host_target:
                        for port in open_ports_for_host[host_target]:
                            res += " " * 4 + "(connected %s %s p%d)\n" % (host_source, host_target, port)
        for vul in self.exploit_actions:
            res += " " * 4 + "(vul_exists %s %s p%d %s)\n" % (vul.CVE, vul.host, vul.port, vul.protocol)
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
    protocol = ""
    risk_factor = "None"
    CVE = ""
    prob = 0
    cost = 0

    # The class "constructor" - It's actually an initializer
    def __init__(self, host, severity, port, protocol, risk_factor, CVE, prob, cost):
        self.host = host
        self.severity = severity
        self.port = port
        self.protocol = protocol
        self.risk_factor = risk_factor
        self.CVE = CVE
        self.prob = prob
        self.cost = cost

    def __str__(self):
        return json.dumps(self.__dict__)


def cvss_metrics_to_prob(cvss_metrics):
    access_complexity = cvss_metrics['access_complexity']
    if access_complexity == 'LOW':
        return 0.8
    elif access_complexity == 'MEDIUM':
        return 0.5
    elif access_complexity == 'HIGH':
        return 0.2


def get_prob_of_CVE (cve):
    cvss_metrics = json.loads(nvd_dict[cve])
    return  cvss_metrics_to_prob(cvss_metrics)


def get_cost_of_CVE (cve):
    return 1


def get_compromised_types_of_CVE(cve):
    cvss_metrics = json.loads(nvd_dict[cve])
    res = []

    confidentiality_impact = cvss_metrics['confidentiality_impact']
    if confidentiality_impact != 'NONE':
        res.append('confidentiality')

    integrity_impact = cvss_metrics['integrity_impact']
    if integrity_impact != 'NONE':
        res.append('integrity')

    availability_impact = cvss_metrics['availability_impact']
    if availability_impact != 'NONE':
        res.append('availability')

    return res




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
    open_ports_for_host[name] = []
    for reportItem in reportHost:
        print(reportItem.tag, reportItem.attrib)
        if (reportItem.tag == 'ReportItem'):
            protocol = reportItem.attrib['protocol']
            severity = int(reportItem.attrib['severity'])
            port = int(reportItem.attrib['port'])
            if not port in uniq_ports:
                uniq_ports.append(port)
            if not port in open_ports_for_host[name]:
                open_ports_for_host[name].append(port)
            if not protocol in protocols:
                protocols.append(protocol)
            ecploit_CVEs = []
            for reportItemChild in reportItem:
                print(reportItemChild.tag, reportItemChild.attrib)
                if (reportItemChild.tag == 'risk_factor'):
                    risk_factor = reportItemChild.text
                if (reportItemChild.tag == 'cve'):
                    ecploit_CVEs.append(reportItemChild.text)
            if severity > 0:
                for cve in ecploit_CVEs:
                    cvss_metrics = json.loads(nvd_dict[cve])
                    print cvss_metrics
                    if(cvss_metrics['access_vector'] != 'LOCAL' and cvss_metrics['integrity_impact'] != 'NONE'):
                        vuln = ExploitAction(name, severity, port, protocol, risk_factor, cve, cvss_metrics_to_prob(cvss_metrics), 1)
                        exploit_actions.append(vuln)
                        if not cve in CVEs:
                            CVEs.append(cve)

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
    f.write(str(PDDLProblem("coresec", "coresec", exploit_actions, hosts[0], goal,
                                "Bla")))



