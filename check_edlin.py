import re

with open('source/modules/mod_edlin.c', 'r') as f1:
    mod_lines = f1.read()
    
with open('source/edlin/edlin.c', 'r') as f2:
    ed_lines = f2.read()

# Let's find functions in edlin.c and compare them to mod_edlin.c
print("File length of edlin.c: ", len(ed_lines))
