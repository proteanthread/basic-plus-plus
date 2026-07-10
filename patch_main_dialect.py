import re

with open('source/core/main.c', 'r') as f:
    lines = f.readlines()

new_lines = []
for line in lines:
    if 'DialectId eff_dialect =' in line:
        continue
    if 'dialect_register_all();' in line:
        continue
    if 'DialectId did = dialect_find_by_name' in line:
        continue
    if 'eff_dialect = did;' in line:
        continue
    if 'eff_dialect = DIALECT_GW_BASIC;' in line:
        continue
    if 'KW_DIALECT' in line:
        continue
    new_lines.append(line)

with open('source/core/main.c', 'w') as f:
    f.writelines(new_lines)
