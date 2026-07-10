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
    
    subsec_match = re.match(r'^### \d+\.\d+\.', line)
    if subsec_match:
        if current_section == 0:
            # We are in a section without a "## Section X" header
            # But the document has no numbered sub-headers before Section 1
            pass
        else:
            sub_section_counts[current_section] += 1
            line = re.sub(r'^### \d+\.\d+\.', f'### {current_section}.{sub_section_counts[current_section]}.', line)

    new_lines.append(line)

with open('README.md', 'w', encoding='utf-8') as f:
    f.writelines(new_lines)
