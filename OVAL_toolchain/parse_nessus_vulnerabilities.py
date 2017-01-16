import xml.etree.ElementTree as ET
import json
import sys


vulnerabilities = []


class Vulnerability(object):
    host = ""
    severity = 1
    port = 0
    risk_factor = "None"

    # The class "constructor" - It's actually an initializer
    def __init__(self, host, severity, port, risk_factor):
        self.host = host
        self.severity = severity
        self.port = port
        self.risk_factor = risk_factor

    def __str__(self):
        return json.dumps(self.__dict__)


tree = ET.parse(sys.argv[1])
root = tree.getroot()
for child in root:
    print(child.tag, child.attrib)
report = root[1]
for reportHost in report:
    print(reportHost.tag, reportHost.attrib)
    name = protocol = reportHost.attrib['name']
    for reportItem in reportHost:
        print(reportItem.tag, reportItem.attrib)
        if (reportItem.tag == 'ReportItem'):
            protocol = reportItem.attrib['protocol']
            severity = reportItem.attrib['severity']
            port = reportItem.attrib['port']
            for reportItemChild in reportItem:
                print(reportItemChild.tag, reportItemChild.attrib)
                if (reportItemChild.tag == 'risk_factor'):
                    risk_factor = reportItemChild.text
            if (int(severity) > 0):
                vuln = Vulnerability(name, severity, port, risk_factor)
                print('bla')
                vulnerabilities.append(vuln)
print '[' + ', '.join(map(str, vulnerabilities)) + ']'

