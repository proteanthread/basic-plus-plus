import re

with open('source/core/main.c', 'r') as f:
    text = f.read()

cb_code = """
// Wrapper to map Edlin's output to the SDL console
static void edlin_write_str_cb(const char *str) {
    gw_printf("%s", str);
}
"""

# Find edlin_execute_buffer_cb and insert before it
idx = text.find("static void edlin_execute_buffer_cb")
if idx != -1:
    text = text[:idx] + cb_code + "\n" + text[idx:]

# Find assignment
old_assign = "edlin_cbs.write_str      = NULL;  /* Use default printf */"
new_assign = "edlin_cbs.write_str      = edlin_write_str_cb;  /* Use GUI printf */"
text = text.replace(old_assign, new_assign)

with open('source/core/main.c', 'w') as f:
    f.write(text)
