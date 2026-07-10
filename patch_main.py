import re

with open('source/core/main.c', 'r') as f:
    text = f.read()

# Find the end of edlin_execute_buffer_cb
end_str = "    exec_run_program(rt, mem);\n}"
if end_str in text:
    text = text.replace(end_str, end_str + "\n\nstatic void edlin_write_str_cb(const char *str) {\n    gw_printf(\"%s\", str);\n}\n")

# Find the assignment
assign_old = "edlin_cbs.write_str      = NULL;  /* Use default printf */"
assign_new = "edlin_cbs.write_str      = edlin_write_str_cb;  /* Use GUI printf */"
if assign_old in text:
    text = text.replace(assign_old, assign_new)

with open('source/core/main.c', 'w') as f:
    f.write(text)
