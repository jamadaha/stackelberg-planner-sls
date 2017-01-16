import xml.etree.ElementTree as ET
import json
import sys


entries = {}


class Entry(object):
    name = ""
    score = 0.0
    access_vector = ""
    access_complexity = ""
    authentication = ""
    confidentiality_impact = ""
    integrity_impact = ""
    availability_impact = ""

    # The class "constructor" - It's actually an initializer
    def __init__(self, name, score, access_vector, access_complexity, authentication, confidentiality_impact, integrity_impact, availability_impact) :
        self.name = name
        self.score = score
        self.access_vector = access_vector
        self.access_complexity = access_complexity
        self.authentication = authentication
        self.confidentiality_impact = confidentiality_impact
        self.integrity_impact = integrity_impact
        self.availability_impact = availability_impact

    def __str__(self):
        return json.dumps(self.__dict__)

class MyEncoder(json.JSONEncoder):
    def default(self, obj):
        if not isinstance(obj, Entry):
            return super(MyEncoder, self).default(obj)

        return json.dumps(obj.__dict__)

tree = ET.parse(sys.argv[1])
root = tree.getroot()
for entry in root:
    print(entry.tag, entry.attrib)
    id = entry.attrib['id']
    for entryChild in entry:
        print(entryChild.tag, entryChild.attrib)
        if entryChild.tag == '{http://scap.nist.gov/schema/vulnerability/0.4}cvss':
            cvss_base_metrics = entryChild[0]
            for base_metric in cvss_base_metrics:
                print(base_metric.tag, base_metric.attrib)
                if base_metric.tag == '{http://scap.nist.gov/schema/cvss-v2/0.2}score':
                    score = base_metric.text
                if base_metric.tag == '{http://scap.nist.gov/schema/cvss-v2/0.2}access-vector':
                    access_vector = base_metric.text
                if base_metric.tag == '{http://scap.nist.gov/schema/cvss-v2/0.2}access-complexity':
                    access_complexity = base_metric.text
                if base_metric.tag == '{http://scap.nist.gov/schema/cvss-v2/0.2}authentication':
                    authentication = base_metric.text
                if base_metric.tag == '{http://scap.nist.gov/schema/cvss-v2/0.2}confidentiality-impact':
                    confidentiality_impact = base_metric.text
                if base_metric.tag == '{http://scap.nist.gov/schema/cvss-v2/0.2}integrity-impact':
                    integrity_impact = base_metric.text
                if base_metric.tag == '{http://scap.nist.gov/schema/cvss-v2/0.2}availability-impact':
                    availability_impact = base_metric.text
            entry = Entry(id, score, access_vector, access_complexity, authentication, confidentiality_impact, integrity_impact, availability_impact)
            entries[id] = entry

with open("nvd.json", "a") as myfile:
    myfile.write(json.dumps(entries, cls=MyEncoder))

