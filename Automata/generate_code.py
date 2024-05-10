import xml.etree.ElementTree as ET

def sanitize_name(name: str):
    return name.replace("[", "").replace("]", "").replace(".","_").upper()

def process_xml_to_c_code(xml_path, output_path):
    # Parse the XML file
    tree = ET.parse(xml_path)
    root = tree.getroot()
    
    # Start writing the C code
    c_code = "#include <stdio.h>\n\n#define IGNORE_EVENT -1\n\n"
    
    for automaton in root.findall('Automaton'):
        name = sanitize_name(automaton.get('name')).lower()
        states = [sanitize_name(state.get('name')) for state in automaton.find('States')]
        events = [sanitize_name(event.get('label')) for event in automaton.find('Events')]
        
        # Create enums for states
        c_code += f"typedef enum {{\n"
        for state in states:
            c_code += f"\t{name}_{state},\n"
        c_code += f"\tNUM_{name}_STATES\n}} State_{name};\n\n"
        
        # Create enums for events
        c_code += f"typedef enum {{\n"
        for event in events:
            c_code += f"\t{name}_EVENT_{event},\n"
        c_code += f"\tNUM_{name}_EVENTS\n}} Event_{name};\n\n"
        
        # Create transition table
        num_states = len(states)
        num_events = len(events)
        c_code += f"State_{name} transition_table_{name}[NUM_{name}_STATES][NUM_{name}_EVENTS];\n\n"
        
        c_code += f"void initialize_transition_table_{name}() {{\n"
        c_code += f"\tfor (int i = 0; i < NUM_{name}_STATES; i++) {{\n"
        c_code += f"\t\tfor (int j = 0; j < NUM_{name}_EVENTS; j++) {{\n"
        c_code += f"\t\t\ttransition_table_{name}[i][j] = IGNORE_EVENT;\n"
        c_code += f"\t\t}}\n"
        c_code += f"\t}}\n\n"
        
        # Fill transition table
        for transition in automaton.find('Transitions'):
            src = states[int(transition.get('source'))]
            dest = states[int(transition.get('dest'))]
            event = events[int(transition.get('event'))]
            c_code += f"\ttransition_table_{name}[{name}_{src}][{name}_EVENT_{event}] = {name}_{dest};\n"
        
        c_code += "}\n\n"

    # Write the C code to a .txt file
    with open(output_path, 'w') as file:
        file.write(c_code)

# Path to the XML file
xml_path = 'statemachine.xml'
# Path to the output C code file
output_path = 'automata_implementation.txt'

# Process the XML and generate the C code
process_xml_to_c_code(xml_path, output_path)
