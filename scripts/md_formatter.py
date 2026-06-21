import os
import re
import sys

def format_md_content(content):
    lines = content.splitlines()
    new_lines = []
    i = 0
    
    is_border = lambda l: len(l) >= 10 and (all(c == '=' for c in l) or all(c == '-' for c in l))
    
    while i < len(lines):
        line = lines[i].strip()
        
        # Detect surrounded border blocks (e.g. === followed by text followed by ===)
        if is_border(line):
            peek_lines = []
            j = i + 1
            while j < len(lines) and not is_border(lines[j].strip()):
                peek_lines.append(lines[j].strip())
                j += 1
                
            if j < len(lines) and is_border(lines[j].strip()):
                # We have a matching closing border!
                header_text = [l for l in peek_lines if l] # Filter out blank lines
                if header_text:
                    # Determine header level
                    # If it's near the top and looks like a title, use H1
                    is_main_title = i < 20 and any(kw in header_text[0].upper() for kw in ["GUIDE", "TUTORIAL", "REFERENCE", "BASIC++", "MANUAL", "SPECIFICATION"])
                    if is_main_title:
                        title = header_text[0]
                        # Clean double spaces or casing
                        new_lines.append(f"# {title}")
                        for extra in header_text[1:]:
                            new_lines.append(f"## {extra}")
                    else:
                        new_lines.append(f"## {' '.join(header_text)}")
                i = j + 1
                continue
            else:
                # No closing border, convert this line to a markdown horizontal rule
                new_lines.append("---")
                i += 1
                continue
        
        # Convert standalone numbered section headers (e.g. "1. OVERVIEW" or "3. BUILD PROFILES")
        # If it matches digit followed by dot and uppercase title
        elif re.match(r'^\d+(\.\d+)?\.\s+[A-Z0-9\s_&/:\-\'\"]+$', line):
            new_lines.append(f"## {line}")
            i += 1
            continue
            
        # Convert standalone subsection headers (e.g. "ESP32-WROOM (no PSRAM):" or "GPIO ACCESS FROM BASIC:")
        elif re.match(r'^[A-Z0-9\s_\-()]+:$', line):
            # Turn into H3
            new_lines.append(f"### {line[:-1]}")
            i += 1
            continue
            
        else:
            new_lines.append(lines[i])
            i += 1
            
    # Clean up multiple consecutive empty lines to keep document compact
    result = []
    consec_empty = 0
    for line in new_lines:
        if line.strip() == "":
            consec_empty += 1
            if consec_empty <= 1:
                result.append(line)
        else:
            consec_empty = 0
            result.append(line)
            
    return "\n".join(result)

def process_docs():
    base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
    docs_dir = os.path.join(base_dir, 'docs')
    
    if not os.path.exists(docs_dir):
        print(f"[ERROR] docs directory not found at: {docs_dir}")
        sys.exit(1)
        
    formatted_count = 0
    skipped_count = 0
    
    for filename in os.listdir(docs_dir):
        # We only touch .md files, and we skip files we know are already formatted properly
        if filename.endswith('.md') and filename not in ['flowchart.md', 'vm_mode.md', 'networking.md']:
            filepath = os.path.join(docs_dir, filename)
            
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                
            # Quick check if it's formatted as plaintext
            if "======" in content or "------" in content or re.search(r'^\d+\.\s+[A-Z]', content, re.MULTILINE):
                print(f"Formatting: {filename}")
                formatted = format_md_content(content)
                
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(formatted)
                formatted_count += 1
            else:
                print(f"Skipping (already MD): {filename}")
                skipped_count += 1
                
    print(f"\nDone! Formatted: {formatted_count} files. Skipped: {skipped_count} files.")

if __name__ == '__main__':
    process_docs()
