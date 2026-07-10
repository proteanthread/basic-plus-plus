import os, re, glob

# 1. Parse lexer.c
lexer_keywords = set()
with open('source/lexer/lexer.c', 'r', encoding='utf-8') as f:
    in_table = False
    for line in f:
        if 'core_keyword_init_table[]' in line:
            in_table = True
        elif in_table and 'dynamic_keyword_table' in line:
            break
        elif in_table:
            match = re.search(r'\{\s*"([^"]+)"', line)
            if match:
                lexer_keywords.add(match.group(1))

# 2. Parse help_*.c files
help_keywords = set()
help_files = glob.glob('source/help/help_*.c')
for file in help_files:
    with open(file, 'r', encoding='utf-8') as f:
        for line in f:
            match = re.search(r'\{\s*"([^"]+)"', line)
            if match:
                help_keywords.add(match.group(1))

# 3. Parse CATALOG.txt
catalog_keywords = set()
with open('CATALOG.TXT', 'r', encoding='utf-8') as f:
    for line in f:
        if not line.startswith('[') and not line.startswith('='):
            words = line.strip().split()
            for word in words:
                catalog_keywords.add(word)

print(f"Total lexer keywords: {len(lexer_keywords)}")
print(f"Total help db keywords: {len(help_keywords)}")
print(f"Total catalog keywords: {len(catalog_keywords)}")

missing_in_help = lexer_keywords - help_keywords
missing_in_catalog = lexer_keywords - catalog_keywords

print(f"\nKeywords in Lexer but not in Help DB ({len(missing_in_help)}):")
print(sorted(list(missing_in_help)))

print(f"\nKeywords in Lexer but not in CATALOG ({len(missing_in_catalog)}):")
print(sorted(list(missing_in_catalog)))

