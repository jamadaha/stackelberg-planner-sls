import argparse
import xml.etree.ElementTree as ET
import json
import sys
from fractions import Fraction
import math
from collections import defaultdict


DIGITS = 2
DENOMINATOR = int(math.pow(10, DIGITS))


def round_float(f):
    return float(("%%.%df" % DIGITS) % f)

PNONE_PROB_DISCOUNT_FACTOR = 0.5

exploit_actions = []
CVEs = []
hosts = []
zones = []
uniq_ports = []
goal = []
open_port_protocol_pairs_for_host = defaultdict(lambda: [])
compromised_types = ['confidentiality', 'integrity', 'availability']
protocols = []
probabilities = []
probabilities_for_CVE = defaultdict(lambda: [])
intial_compromised_predicates = ""
initial_haclz_predicates = ""
initial_subnet_predicates = ""


def main():
    global nvd_dict

    p = argparse.ArgumentParser(description="Nessus CoreSec Problem Generator")
    p.add_argument("--nessus", type=str, help="the Nessus output file in xml format", default=None)
    p.add_argument("--nvd", type=str, help="NVD vulnerability DB in json format", default=None)
    p.add_argument("--net", type=str, help="The network topology in PDDL format", default=None)
    p.add_argument("--fix", type=str, help="The Fix-Action set description in json format", default=None)
    p.add_argument("--disable-apply-once", help="disable apply once constraint", action="store_true", default=False)
    p.add_argument("--domain", type=str, help="path to file where the PDDL domain will be stored", default="domain.pddl")
    p.add_argument("--problem", type=str, help="path to file where the PDDL problem will be stored", default="problem.pddl")
    args = p.parse_args(sys.argv[1:])


    with open(args.nvd) as nvd_file:
        nvd_dict = json.load(nvd_file)

    parse_nessus_report(args)
    parse_network_topology_file(args)

    if args.fix is not None:
        with open(args.fix) as fix_actions_file:
            fix_actions = parse_fix_actions_file(fix_actions_file)
    with open(args.domain, "w") as f:
        f.write(generate_PDDL_domain("coresec", args, fix_actions, "Bla", apply_once=not args.disable_apply_once))

    with open(args.problem, "w") as f:
        f.write(generate_PDDL_problem("coresec", "coresec", "Bla"))


class ExploitAction(object):
    host = ""
    severity = 1
    port = "pnone"
    protocol = ""
    CVE = ""
    prob = 0
    cost = 0

    # The class "constructor" - It's actually an initializer
    def __init__(self, host, severity, port, protocol, CVE, prob, cost):
        self.host = host
        self.severity = severity
        self.port = port
        self.protocol = protocol
        self.CVE = CVE
        self.prob = prob
        self.cost = cost

    def __str__(self):
        return json.dumps(self.__dict__)


def generate_PDDL_domain(name, args, fix_actions = None, comment="", apply_once=True):
    res = ''
    if comment != None:
        res += ';;; %s\n' % comment
    res += "(define (domain %s)\n" % name
    res += "; (:requirements strips)\n"
    res += "(:types host zone port protocol vul compromised_type probability"
    res += ")\n"
    res += "(:predicates\n"
    res += " " * 3
    for i in range(len(goal)):
        res += " (target%d_under_control)" % i
    res += "\n"
    res += " " * 4 + "(compromised ?h - host ?ct - compromised_type)\n"
    # res += " " * 4 + "(hacl ?src ?dest - host ?po - port ?pr - protocol)\n"
    res += " " * 4 + "(haclz ?src ?dest - zone ?po - port ?pr - protocol)\n"
    res += " " * 4 + "(subnet ?z - zone ?h - host)\n"
    res += " " * 4 + "(vul_exists ?v - vul ?h - host ?po - port ?pr - protocol ?prob - probability)\n"
    res += " " * 4 + "(dpf ?src - zone ?h - host ?po - port ?pr - protocol)\n"
    if apply_once:
        res += " " * 4 + "(applied ?v - vul ?h - host)\n"
    res += ")\n"
    res += "(:functions (total-cost))\n"
    res += "(:constants\n"
    res += " " * 4
    for i in range(0, len(hosts)):
        res += "%s " % hosts[i]
    res += "- host\n"
    res += " " * 4
    for p in uniq_ports:
        res += "%s " % p
    res += "- port\n"
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
    res += " " * 4
    for prob in probabilities:
        res += "prob%s " % prob
    res += "- probability\n"
    res += " " * 4
    for zone in zones:
        res += "%s " % zone
    res += "- zone\n"
    res += ")\n"
    for i in range(len(goal)):
        for host in goal[i]:
            res += "(:action ATTACK_set_target%d_under_control_%s_1/1\n" % (i, host)
            res += " " * 4 + ":parameters ()\n"
            res += " " * 4 + ":precondition (and (not (target%d_under_control)) (compromised %s integrity))\n" % (
            i, host)
            res += " " * 4 + ":effect (and (target%d_under_control) (increase (total-cost) 0))\n" % i
            res += ")\n"
    for cve in CVEs:
        for prob in (probabilities_for_CVE[cve] + probabilities_for_CVE['*']):
            f = Fraction(round_float(float(prob))).limit_denominator(DENOMINATOR)
            res += "(:action ATTACK_exploit_%s_%s\n" % (cve, f)
            res += " " * 4 + ":parameters (?src ?t - host ?z1 %s- zone ?po - port ?pr - protocol)\n" % (
            "?z2 " if get_access_vector_of_CVE(cve) == "NETWORK" else "")
            res += " " * 4 + ":precondition (and (compromised ?src integrity)"
            if get_access_vector_of_CVE(cve) == "NETWORK":
                res += " (subnet ?z1 ?src)"
                res += " (subnet ?z2 ?t)"
                res += " (haclz ?z1 ?z2 ?po ?pr)"
                res += " (not (dpf ?z1 ?t ?po ?pr))"
            elif get_access_vector_of_CVE(cve) == "ADJACENT_NETWORK":
                res += " (subnet ?z1 ?src)"
                res += " (subnet ?z1 ?t)"
                res += " (not (dpf ?z1 ?t ?po ?pr))"
            else:
                print "Should not end up here!"
                exit()
            res += " (vul_exists %s ?t ?po ?pr prob%s)" % (cve, prob)

            if apply_once:
                res += " (not (applied %s ?t))" % cve
            res += ")\n"
            res += " " * 4 + ":effect (and (increase (total-cost) %d)" % get_cost_of_CVE(cve)
            for ct in get_compromised_types_of_CVE(cve):
                res += " (compromised ?t %s)" % ct
            if apply_once:
                res += " (applied %s ?t)" % cve
            res += ")\n"
            res += ")\n"

    if fix_actions is not None:
        res += fix_actions
    else:
        fix_action_scheme_id = 0
        for cve in CVEs:
            initial_cost = 1
            cost = 1
            res += "(:action FIX_exploit_%s_%d#%d\n" % (cve, initial_cost, fix_action_scheme_id)
            res += " " * 4 + ":parameters (?h - host ?po - port ?pr - protocol ?prob - probability)\n"
            res += " " * 4 + ":precondition (and (vul_exists %s ?h ?po ?pr ?prob)" % cve
            res += ")\n"
            res += " " * 4 + ":effect (and (increase (total-cost) %d) (not (vul_exists %s ?h ?po ?pr ?prob))" % (
            cost, cve)
            res += ")\n"
            res += ")\n"
            fix_action_scheme_id += 1
    res += ")\n"
    return res


def generate_PDDL_problem(name, domain_name, comment=""):
    res = ''
    if comment != None:
        res += ";;; %s\n" % comment
    res += "(define (problem %s)\n" % name
    res += "(:domain %s)\n" % domain_name
    res += "(:init\n"
    res += " " * 4 + "(= (total-cost) 0)\n"
    res += intial_compromised_predicates
    res += initial_subnet_predicates
    res += initial_haclz_predicates

    for vul in exploit_actions:
        res += " " * 4 + "(vul_exists %s %s %s %s prob%s)\n" % (vul.CVE, vul.host, vul.port, vul.protocol, vul.prob)
    res += ")\n"
    res += "(:goal (and\n"
    for i in range(len(goal)):
        res += " " * 4 + "(target%d_under_control)\n" % i
    res += "))\n"
    res += "(:metric minimize (total-cost))\n"
    res += ")\n"
    return res


def parse_nessus_report(args):
    tree = ET.parse(args.nessus)
    root = tree.getroot()
    for child in root:
        print(child.tag, child.attrib)
    report = root[1]
    for reportHost in report:
        print(reportHost.tag, reportHost.attrib)
        name = reportHost.attrib['name']
        hosts.append(name)
        open_port_protocol_pairs_for_host[name] = []
        for reportItem in reportHost:
            print(reportItem.tag, reportItem.attrib)
            if (reportItem.tag == 'ReportItem'):
                port = reportItem.attrib['port']
                protocol = reportItem.attrib['protocol']
                severity = int(reportItem.attrib['severity'])
                if port == "0":
                    port = "pnone"
                else:
                    port = "p" + port
                port_protocol_pair = (port, protocol)

                if not port in uniq_ports:
                    uniq_ports.append(port)
                if not port_protocol_pair in open_port_protocol_pairs_for_host[name]:
                    open_port_protocol_pairs_for_host[name].append(port_protocol_pair)
                ecploit_CVEs = []
                for reportItemChild in reportItem:
                    print(reportItemChild.tag, reportItemChild.attrib)
                    if (reportItemChild.tag == 'cve'):
                        ecploit_CVEs.append(reportItemChild.text)
                if severity > 0:
                    for cve in ecploit_CVEs:
                        cvss_metrics = json.loads(nvd_dict[cve])
                        print cvss_metrics
                        if(cvss_metrics['access_vector'] != 'LOCAL' and cvss_metrics['integrity_impact'] != 'NONE'):
                            if not protocol in protocols:
                                protocols.append(protocol)
                            prob = cvss_metrics_to_prob(cvss_metrics)
                            if port == "pnone":
                                prob = prob * PNONE_PROB_DISCOUNT_FACTOR
                            if not prob in probabilities:
                                probabilities.append(prob)
                            if prob not in probabilities_for_CVE[cve]:
                                probabilities_for_CVE[cve].append(prob)
                            vuln = ExploitAction(name, severity, port, protocol, cve, prob, 1)
                            exploit_actions.append(vuln)
                            if not cve in CVEs:
                                CVEs.append(cve)
    #print '[' + ', '.join(map(str, exploit_actions)) + ']'


def parse_fix_actions_file(fix_actions_file):
    fixes = ""
    fix_actions_json = json.load(fix_actions_file)
    fix_action_scheme_id = 0
    for fix_action_scheme in fix_actions_json:
        type = fix_action_scheme['type']
        if type == "FIX":
            cve = fix_action_scheme['CVE']
            host = fix_action_scheme['host']
            port = fix_action_scheme['port']
            protocol = fix_action_scheme['protocol']
            new_prob = float(fix_action_scheme['new_prob'])
            initial_cost = fix_action_scheme['initial_cost']
            cost = fix_action_scheme['cost']
            if new_prob > 0.0 and new_prob not in probabilities:
                probabilities.append(new_prob)
            if new_prob > 0.0 and new_prob not in probabilities_for_CVE[cve]:
                probabilities_for_CVE[cve].append(new_prob)
            fixes += "(:action FIX_exploit_%s%s%s%s%s%d#%d\n" % (
                "" if cve == '*' else cve + "_", "" if host == '*' else host + "_", "" if port == '*' else port + "_",
                "" if protocol == '*' else protocol + "_", "prob" + str(new_prob) + "_", initial_cost,
                fix_action_scheme_id)
            fixes += " " * 4 + ":parameters (%s%s%s%s?old_prob - probability)\n" % (
            "?v - vul " if cve == '*' else "", "?h - host " if host == '*' else "",
            "?po - port " if port == '*' else "",
            "?pr - protocol " if protocol == '*' else "")
            fixes += " " * 4 + ":precondition (and (vul_exists %s %s %s %s ?old_prob)" % (
            "?v" if cve == '*' else cve, "?h" if host == '*' else host, "?po" if port == '*' else port,
            "?pr" if protocol == '*' else protocol)
            fixes += ")\n"
            fixes += " " * 4 + ":effect (and (increase (total-cost) %d)" % cost
            fixes += " (not (vul_exists %s %s %s %s ?old_prob))" % (
            "?v" if cve == '*' else cve, "?h" if host == '*' else host, "?po" if port == '*' else port,
            "?pr" if protocol == '*' else protocol)
            if new_prob > 0.0:
                fixes += " (vul_exists %s %s %s %s %s)" % (
                "?v" if cve == '*' else cve, "?h" if host == '*' else host, "?po" if port == '*' else port,
                "?pr" if protocol == '*' else protocol, "prob" + str(new_prob))
            fixes += ")\n"
            fixes += ")\n"
        elif type == "ZONE-FW":
            src = fix_action_scheme['src_zone']
            dest = fix_action_scheme['dest_zone']
            port = fix_action_scheme['port']
            protocol = fix_action_scheme['protocol']
            initial_cost = fix_action_scheme['initial_cost']
            cost = fix_action_scheme['cost']
            fixes += "(:action FIX_install_zone_firewall_%s%s%s%s%d#%d\n" % (
            "" if src == '*' else src + "_", "" if dest == '*' else dest + "_", "" if port == '*' else port + "_",
            "" if protocol == '*' else protocol + "_", initial_cost, fix_action_scheme_id)
            fixes += " " * 4 + ":parameters (%s%s%s%s)\n" % (
            "?src - zone " if src == '*' else "", "?dest - zone " if dest == '*' else "",
            "?po - port " if port == '*' else "",
            "?pr - protocol" if protocol == '*' else "")
            fixes += " " * 4 + ":precondition (and (haclz %s %s %s %s)" % (
            "?src" if src == '*' else src, "?dest" if dest == '*' else dest, "?po" if port == '*' else port,
            "?pr" if protocol == '*' else protocol)
            fixes += ")\n"
            fixes += " " * 4 + ":effect (and (increase (total-cost) %d)" % cost
            fixes += " (not (haclz %s %s %s %s))" % (
            "?src" if src == '*' else src, "?dest" if dest == '*' else dest, "?po" if port == '*' else port,
            "?pr" if protocol == '*' else protocol)
            fixes += ")\n"
            fixes += ")\n"
        elif type == "DESKTOP-FW":
            src = fix_action_scheme['src_zone']
            host = fix_action_scheme['host']
            port = fix_action_scheme['port']
            protocol = fix_action_scheme['protocol']
            initial_cost = fix_action_scheme['initial_cost']
            cost = fix_action_scheme['cost']
            fixes += "(:action FIX_install_desktop_firewall_%s%s%s%s%d#%d\n" % (
            "" if src == '*' else src + "_", "" if host == '*' else host + "_", "" if port == '*' else port + "_",
            "" if protocol == '*' else protocol + "_", initial_cost, fix_action_scheme_id)
            fixes += " " * 4 + ":parameters (%s%s%s%s)\n" % (
            "?src - zone " if src == '*' else "", "?h - host " if host == '*' else "",
            "?po - port " if port == '*' else "",
            "?pr - protocol" if protocol == '*' else "")
            # fixes += " " * 4 + ":precondition ( "
            # fixes += ")\n"
            fixes += " " * 4 + ":effect (and (increase (total-cost) %d)" % cost
            fixes += " (dpf %s %s %s %s)" % (
            "?src" if src == '*' else src, "?h" if host == '*' else host, "?po" if port == '*' else port,
            "?pr" if protocol == '*' else protocol)
            fixes += ")\n"
            fixes += ")\n"
        fix_action_scheme_id += 1
    return fixes


def parse_network_topology_file(args):
    global intial_compromised_predicates
    global initial_haclz_predicates
    global initial_subnet_predicates
    with open(args.net) as network_file:
        network_json = json.load(network_file)
        print network_json
        for zone in network_json:
            print zone
            zone_name = zone['zone_name']
            host_list = zone['hosts']
            if 'integrity_initially_compromised' in zone:
                integrity_initially_compromised = zone['integrity_initially_compromised']
            else:
                integrity_initially_compromised = 0
            if 'is_goal' in zone:
                is_goal = zone['is_goal']
            else:
                is_goal = 0
            allowed_incoming_rules = zone['allowed_incoming_rules']
            if is_goal == 1:
                goal.append(host_list)
            open_port_protocol_pairs_in_this_zone = []
            if zone_name not in zones:
                zones.append(zone_name)
            for host in host_list:
                if host not in hosts:
                    hosts.append(host)
                initial_subnet_predicates += " " * 4 + "(subnet %s %s)\n" % (zone_name, host)
                open_port_protocol_pairs_in_this_zone += open_port_protocol_pairs_for_host[host]
                if integrity_initially_compromised == 1:
                    intial_compromised_predicates += " " * 4 + "(compromised %s integrity)\n" % host
            for port_protocol_pair in open_port_protocol_pairs_in_this_zone:
                initial_haclz_predicates += " " * 4 + "(haclz %s %s %s %s)\n" % (
                zone_name, zone_name, port_protocol_pair[0], port_protocol_pair[1])
            for rule in allowed_incoming_rules:
                src_zone_name = rule['zone']
                port = str(rule['port'])
                protocol = rule['protocol']
                if port == "0":
                    port = "pnone"
                else:
                    port = "p" + port
                initial_haclz_predicates += " " * 4 + "(haclz %s %s %s %s)\n" % (
                src_zone_name, zone_name, port, protocol)


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
    return cvss_metrics_to_prob(cvss_metrics)


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


def get_access_vector_of_CVE(cve):
    cvss_metrics = json.loads(nvd_dict[cve])
    return cvss_metrics['access_vector']


main()
