The python script what-if-analysis.py is a tool for automated what-if analysis
via simulated network pentesting.

The tool builds a formal attacker model, and determines the Pareto-optimal
frontier of fix action sequences which minimize the attacker success
probability, where the attacker success probability depends on the exploit
actions at his disposal.

Attacker model
==============

The attacker initially controls a specific set of host in the network and has
the goal to control another set of hosts. The attacker gains control over a new
host t by already controlling another host s and using an applicable exploit
action which uses a vulnerability of t.  Applicability also includes the
requirement that the connection from s to t is not blocked by a firewall/packet
filter.

Fix Actions
===========

Additionally, there exists a set of fix actions, specified via a separate input
file. In general, these actions change the network so that some attacker
actions are disabled. A fix action might model, e.g., patching a vulnerability
on a specific host (vulnerabilities are identified via CVE ids) or adding
a firewall rule, see below for more details.

Installation
============

To build the tool, navigate to the /src folder and execute:
./build_all

Command line arguments
======================

The script what-if-analysis.py basically needs four input files given via the following cmd arguments
    --nessus: One or several Nessus network scan result files in .xml format
    --nvd: A .json file in our own format which basically contains important
           information (cvss-metrics) extracted from XML Vulnerability Feeds
           (Download: https://nvd.nist.gov/download.cfm). This repository provides 
           "nvd_2002-2017.json" as an example. So, no additional work for you,
           just give this file as argument.
    --fix A .json filne in our own format which describes the possible fix actions (more below)
    --net A .json filne in our own format which describes the topology of the network belonging to the nessus scan

The following three arguments are optional
	--exploit-infos: a json file containting an array with objects that each specify a probability and cost for a CVE id. If no manual infos are provided for an CVE id, the prob. and cost are extracted from the nvd file. One can also define completely new exploits.
	--initial-attack-budget: The budget for the attacker
	--initial-fix-budget: The budget for the fix actions

Example
=======

The tool can be started with the example files by executing the following
command (outside src, i.e., in the top-level directory).

python what-if-analysis.py --nessus Nessus_toolchain/Network_Scan_example.nessus\
                           --nvd Nessus_toolchain/nvd_2002-2017.json\
                           --fix Nessus_toolchain/fix_actions_description_example.json\
                           --net Nessus_toolchain/network_topology_example.json

Fix actions description file format
===================================

The .json file must contain a single array filled with objects, all specifying a specific fix action schema. Such an Object can have three possible types:
"FIX" is for a patch or workaround for all of specific vulnerabilites.
{
	"type": "FIX"
	"CVE": "x", // x can either be "*" or a specific CVE id meaning that a fix action scheme is created which in principle could fix all CVEs ("*") or only this specific one
	"host": "x", // x can either be "*" or a specific host name meaning that a fix action scheme is created which in principle could fix the vulnerability on all hosts or only on this specific one
	"port": "x", // x can either be "*" or a specific port meaning that a fix action scheme is created which in principle could fix the vulnerability running on all ports or only on this specific one
	"protocol": "x", // x can either be "*" or a specific protocol (e.g. "tcp", "udp") meaning that a fix action scheme is created which in principle could fix the vulnerability running on all protocols or only on this specific one
	"new_prob": "x", // x is the new probability of the respective exploit action. If the exploit is not applicable anymore, it is "0". Note that new_prob must be smaller than the original probability. The original probability is extracted from the nvd file.
	"initial_cost": x, // As this object results in an action schema which will be instantiated to several conrecte fix actions, (e.g. if host: "*", there is concrete action for every host) we want to reflect the cost of initially finding the patch for an CVE id. The initial_cost is only accounted once, even if several fix actions from this scheme are in a sequence.
	"cost": x // The normal costs are always accounted for each individual fix action in a sequence
} 

 "ZONE-FW" is for adding a firewall rule between ZONEs (subnets):
{
	"type": "ZONE-FW",
	"src_zone": "x", // x can either be "*" or a zone name meaning that a fix action scheme is created which in principle blocks access from all zones ("*") or only from a specific one
	"dest_zone": "x", // x can either be "*" or a zone name meaning that a fix action scheme is created which in principle blocks access to all zones ("*") or only to a specific one
	"port": "x", // x can either be "*" or a port meaning that a fix action scheme is created which in principle blocks access to all ports ("*") or only to a specific one
	"protocol": "*", // x can either be "*" or a protocol meaning that a fix action scheme is created which in principle blocks access for all protocols ("*") or for a specific one
	"initial_cost": x, // as already said, the initial costs are only accountend once for this scheme
	"cost": x // these costs are always accounted
}

"DESKTOP-FW" is for adding a firewall rule on a specific host:
{
	"type": "DESKTOP-FW",
	"src_zone": "*", // x can either be "*" or a zone name meaning that a fix action scheme is created which in principle blocks access from all zones ("*") or only from a specific one
	"host": "*", // x can either be "*" or a host name meaning that a fix action scheme is created which in principle blocks access to all host ("*") or only to a specific one
	"port": "*", // x can either be "*" or a port meaning that a fix action scheme is created which in principle blocks access to all ports ("*") or only to a specific one
	"protocol": "*", // x can either be "*" or a protocol meaning that a fix action scheme is created which in principle blocks access for all protocols ("*") or for a specific one
	"initial_cost": 1, // initial costs are only accountend once for this scheme
	"cost": 1 // these costs are always accounted
}


Network topology description file format
========================================

The .json file must contain a single array filled with objects, all specifying a zone aka subnet.
Such an Object looks as follows:
{
	"zone_name": "x", // x is the name of this zone
	"hosts": ["x", "y"], // this is an array of host names which are in this zone
	"integrity_initially_compromised": x, // x can be either 0 or 1 indicating whether the attacker initially controlls the hosts in this network or not
	"is_goal_confidentiality": x, // x can be either 0 or 1 indication whether the attacker's goal entails that he must compromise the confidentiality of at least one host from this zone
	"is_goal_integrity": x, // x can be either 0 or 1 indication whether the attacker's goal entails that he must compromise the integrity of at least one host from this zone
	"is_goal_availability": x, // x can be either 0 or 1 indication whether the attacker's goal entails that he must compromise the availability of at least one host from this zone
	"allowed_incoming_rules": // this is an array of firewall rules indicating which incoming connections are allowed and not blocked
	[
		{					// this is an object representing an allowed incoming connection
			"zone": "x",	// x is the name of the zone which is granted access
			"port": "x",	// x is the port on which the connection is allowed
			"protocol": "x"	// x is the protocol (e.g. tcp, udp) which is allowed
		}
	]
}

Exploit infos description file format
=====================================

The .json file must contain a single array filled with objects, all specifying properties of an existing CVE id or of a completely new exploit with an unique id.

To change properties of an existing exploit, such an Object looks as follows:
{
	"CVE": "x", // x is an CVE id
	"prob": x, // (optional) x is the probability that this exploit is successful as decimal number (e.g. 0.5)
	"cost": x // (optional) x is the attacker's cost for executing this exploit as integer number (e.g. 42)
}

To define a new exploit, such an Object looks as follows:
{
	"id": "x", // x is a new unique id, similar to CVE ids
	"access_vector": "x", can be either "NETWORK" or "ADJACENT_NETWORK"
	"prob": x, // x is the probability that this exploit is successful as decimal number (e.g. 0.5)
	"cost": x, //  x is the attacker's cost for executing this exploit as integer number (e.g. 42)
	"compromise": [], // the array contains a non-empty subset of the following compromise-types: "confidentiality", "integrity" and "availability"
	"hosts": [], // the array contains a non-empty list of host names on which the exploit is applicable e.g. ("10.9.59.118")
	"port": "x", // x is the port on which the connection between soruce and target hosts must be allowed to execute the exploit
	"protocol": "x" // x is the protocol (e.g. tcp, udp) which the must be allowed for a connection between soruce and target hosts on the already specified port
}


