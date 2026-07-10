import difflib

with open('source/modules/mod_edlin.c', 'r') as f1:
    mod_lines = f1.readlines()
    
with open('source/edlin/edlin.c', 'r') as f2:
    ed_lines = f2.readlines()

d = difflib.unified_diff(mod_lines, ed_lines, fromfile='mod_edlin.c', tofile='edlin.c')
diff_output = ''.join(d)

with open('diff_output.txt', 'w') as out:
    out.write(diff_output)
