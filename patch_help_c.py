import re

with open('source/help/help.c', 'r', encoding='utf-8') as f:
    text = f.read()

# Add externs
externs = """extern const HelpEntry help_db_progmgmt[];
extern const HelpEntry help_db_testing[];
extern const HelpEntry help_db_devices[];
"""
text = text.replace('extern const HelpEntry help_db_oper[];', 'extern const HelpEntry help_db_oper[];\n' + externs)

# Add to help_dbs array
dbs = """    help_db_progmgmt,
    help_db_testing,
    help_db_devices,"""
text = text.replace('    help_db_oper,\n    NULL', '    help_db_oper,\n' + dbs + '\n    NULL')

with open('source/help/help.c', 'w', encoding='utf-8') as f:
    f.write(text)
