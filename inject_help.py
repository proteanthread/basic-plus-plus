import re

def insert_help(filename, content):
    with open(filename, 'r') as f:
        text = f.read()
    text = text.replace('  { NULL, NULL, NULL, 0 }', content + '\n  { NULL, NULL, NULL, 0 }')
    with open(filename, 'w') as f:
        f.write(text)

insert_help('source/help/help_io.c', '  { "INPUT", "Get console input into a variable", "INPUT \\"Name? \\"; N$", HCAT_IO },\n  { "AT", "Position cursor (used with PRINT)", "PRINT AT(10,5); \\"Hello\\"", HCAT_IO },')
insert_help('source/help/help_sysenv.c', '  { "DATE", "Get or set the system date", "D$ = DATE$", HCAT_SYSENV },\n  { "TIME", "Get or set the system time", "T$ = TIME$", HCAT_SYSENV },\n  { "SYS", "Execute a system command", "SYS \\"dir\\"", HCAT_SYSENV },\n  { "EXEC", "Execute a system command (Alias)", "EXEC \\"ls\\"", HCAT_SYSENV },\n  { "SHELL", "Drop to system shell or exec command", "SHELL", HCAT_SYSENV },\n  { "KEYWORD", "Modify keyword attributes at runtime", "KEYWORD PRINT UPPERCASE ON", HCAT_SYSENV },')
insert_help('source/help/help_varmem.c', '  { "DATA", "Store numeric or string data", "DATA 10, 20, \\"Hello\\"", HCAT_VARMEM },\n  { "PSTORE", "Store variable persistently", "PSTORE A", HCAT_VARMEM },\n  { "PRETRIEVE", "Retrieve persistent variable", "PRETRIEVE A", HCAT_VARMEM },')
insert_help('source/help/help_flow.c', '  { "CALL", "Call a SUB procedure", "CALL MySub(1, 2)", HCAT_FLOW },\n  { "TASK", "Multi-tasking thread spawn", "TASK 100", HCAT_FLOW },')
insert_help('source/help/help_math.c', '  { "NUM", "Convert string to number", "N = NUM(\\"123\\")", HCAT_MATH },')
