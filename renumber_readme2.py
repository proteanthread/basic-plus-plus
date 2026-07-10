import re

with open('README.md', 'r', encoding='utf-8') as f:
    lines = f.readlines()

new_lines = []
current_section = 0
sub_section_counts = {}

for line in lines:
    sec_match = re.match(r'^## Section (\d+):', line)
    if sec_match:
        current_section += 1
        line = re.sub(r'^## Section \d+:', f'## Section {current_section}:', line)
        sub_section_counts[current_section] = 0
    
    # Check for ### allowing leading spaces, but strip them in replacement
    subsec_match = re.match(r'^\s*### \d+\.\d+\.', line)
    if subsec_match:
        if current_section == 0:
            pass
        else:
            sub_section_counts[current_section] += 1
            line = re.sub(r'^\s*### \d+\.\d+\.', f'### {current_section}.{sub_section_counts[current_section]}.', line)

    new_lines.append(line)

with open('README.md', 'w', encoding='utf-8') as f:
    f.writelines(new_lines)
