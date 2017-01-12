import xml.etree.ElementTree as ET

def_if_to_formal = {}

def parse_criteria_in_inventory_def(criteria):
    ret_value = ''
    if 'operator' in criteria.attrib:
        op = criteria.attrib['operator']
    else:
        op = 'AND'
    ret_value += '(' + op + ' '
    if 'test_ref' in criteria.attrib:
        ret_value += '(' + criteria.attrib['test_ref'] + ')'
    for child in criteria:
        ret_value += parse_criteria_in_inventory_def(child)
    ret_value += ')'
    return ret_value

def parse_criteria_in_vulnerability_def(criteria):
    ret_value = ''
    if 'operator' in criteria.attrib:
        op = criteria.attrib['operator']
    else:
        op = 'AND'
    ret_value += '('
    if len(criteria) > 0:
        ret_value += op + ' '
    if 'test_ref' in criteria.attrib:
        ret_value += '(' + criteria.attrib['test_ref'] + ')'
    if 'definition_ref' in criteria.attrib:
        ret_value += '(' + def_if_to_formal[criteria.attrib['definition_ref']] + ')'
    for child in criteria:
        ret_value += parse_criteria_in_vulnerability_def(child)

    ret_value += ')'
    return ret_value


tree = ET.parse('unix.xml')
root = tree.getroot()
for child in root:
    print(child.tag, child.attrib)

definitions = root[1]
for definition in definitions:
    print(definition.tag, definition.attrib)
    if (definition.attrib['class'] == 'inventory'):
        id = definition.attrib['id']
        criteria = definition.find('{http://oval.mitre.org/XMLSchema/oval-definitions-5}criteria')
        print(criteria.tag, criteria.attrib)
        parsed_criteria = parse_criteria_in_inventory_def(criteria)
        def_if_to_formal[id] = parsed_criteria
        print(parsed_criteria)
    elif (definition.attrib['class'] == 'vulnerability'):
        criteria = definition.find('{http://oval.mitre.org/XMLSchema/oval-definitions-5}criteria')
        print(criteria.tag, criteria.attrib)
        print(parse_criteria_in_vulnerability_def(criteria))


