import os
import re
import glob

# Map C file to category name
cat_names = {
    'help_flow.c': 'Control Flow',
    'help_io.c': 'Input / Output',
    'help_math.c': 'Arithmetic / Math',
    'help_strings.c': 'String Functions',
    'help_varmem.c': 'Variables & Memory',
    'help_fileio.c': 'File I/O',
    'help_gfx.c': 'Graphics',
    'help_sound.c': 'Sound',
    'help_sysenv.c': 'System & Environ',
    'help_testing.c': 'Debug & Testing',
    'help_progmgmt.c': 'Program Mgmt & Editing',
    'help_devices.c': 'Devices & Network',
    'help_misc.c': 'Operators',
    'help_intro.c': 'Introspection',
    'help_sysvar.c': 'System Variables'
}

db = {}

files = glob.glob('source/help/help_*.c')
for file in files:
    cat = cat_names.get(os.path.basename(file), 'Uncategorized')
    if cat not in db:
        db[cat] = []
    
    with open(file, 'r', encoding='utf-8') as f:
        for line in f:
            match = re.search(r'\{\s*"([^"]+)",\s*"([^"]+)",\s*"([^"]+)"', line)
            if match:
                kw, desc, usage = match.groups()
                db[cat].append((kw, desc, usage))

# Sort everything alphabetically
for cat in db:
    db[cat].sort(key=lambda x: x[0])

# Generate CATALOG.TXT
with open('CATALOG.TXT', 'w', encoding='utf-8') as f:
    f.write("=== BASIC++ CATALOG ===\n\n")
    for cat in sorted(db.keys()):
        f.write(f"[{cat}]\n")
        kws = [x[0] for x in db[cat]]
        col_width = 15
        for i in range(0, len(kws), 5):
            row = kws[i:i+5]
            f.write(" " + "".join(w.ljust(col_width) for w in row) + "\n")
        f.write("\n")

# Generate HELP.txt
with open('HELP.txt', 'w', encoding='utf-8') as f:
    f.write("=== BASIC++ COMMAND REFERENCE ===\n\n")
    for cat in sorted(db.keys()):
        f.write(f"[{cat}]\n")
        for kw, desc, usage in db[cat]:
            f.write(f" {kw.ljust(12)} {desc}\n")
        f.write("\n")

# Generate help/quick_reference.TXT
with open('help/quick_reference.TXT', 'w', encoding='utf-8') as f:
    f.write("BASIC++ Quick Reference\n\n")
    for cat in sorted(db.keys()):
        f.write(f"{cat.replace(' ', '')}\n")
        for kw, desc, usage in db[cat]:
            f.write(f"- {kw}\n")
        f.write("\n")

# Generate docs/quick_reference.md
os.makedirs('docs', exist_ok=True)
with open('docs/quick_reference.md', 'w', encoding='utf-8') as f:
    f.write("# BASIC++ Quick Reference\n\n")
    for cat in sorted(db.keys()):
        f.write(f"## {cat}\n")
        for kw, desc, usage in db[cat]:
            f.write(f"- **{kw}** - {desc}\n")
        f.write("\n")

print("Generated documentation files successfully!")
