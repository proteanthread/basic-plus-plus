import re

with open('source/core/main.c', 'r') as f:
    text = f.read()

# Remove the Dialect block
dialect_block_start = text.find("// Dialect\n if (cfg.found && cfg.dialect[0] != '\\0') {")
dialect_block_end = text.find(" if (g_cli_lite) {", dialect_block_start)

if dialect_block_start != -1 and dialect_block_end != -1:
    text = text[:dialect_block_start] + text[dialect_block_end:]

# Also check for other dialect uses
text = re.sub(r'boot_cfg.dialect\s*=\s*eff_dialect;', '', text)
text = re.sub(r'if \(!eff_quiet\) \{[\s\S]*?printf\("Dialect: %s\\n", dialect_get_name\(eff_dialect\)\);[\s\S]*?\}', '', text)
text = re.sub(r'printf\("Dialect: %s\\n", dialect_get_name\(eff_dialect\)\);', '', text)
text = re.sub(r'printf\("  -d DIALECT       Set default dialect \(GWBS, QBAS, PATB, BPP, etc.\)\\n"\);', '', text)

with open('source/core/main.c', 'w') as f:
    f.write(text)
