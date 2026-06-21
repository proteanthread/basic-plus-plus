import os
import re

def strip_markdown(md_text):
    # 1. Strip fenced code block markers (e.g. ```basic ... ```)
    text = re.sub(r'^[ \t]*```\w*[ \t]*$', '', md_text, flags=re.MULTILINE)
    
    # 2. Strip headers prefix (# Title -> Title)
    text = re.sub(r'^[ \t]*#+\s+(.*)$', r'\1', text, flags=re.MULTILINE)
    
    # 3. Strip bold and italic markup safely without matching glob wildcards or underscores inside words
    text = re.sub(r'\*\*((?:[^\n*]|\n(?!\n))+?)\*\*', r'\1', text)
    text = re.sub(r'\*([^\n*./\\]+?)\*', r'\1', text)
    text = re.sub(r'\b_([^\n_]+?)_\b', r'\1', text)
    
    # 4. Strip inline code backticks (`code` -> code)
    text = re.sub(r'`([^`\n]+?)`', r'\1', text)
    
    # 5. Convert links: [text](url) -> text
    text = re.sub(r'\[([^\]\n]+?)\]\([^)\n]+?\)', r'\1', text)
    
    # 6. Convert images: ![alt](url) -> alt
    text = re.sub(r'!\[([^\]\n]+?)\]\([^)\n]+?\)', r'\1', text)
    
    return text

def sync_docs_to_help():
    base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
    docs_dir = os.path.join(base_dir, 'docs')
    help_dir = os.path.join(base_dir, 'help')
    
    if not os.path.exists(help_dir):
        os.makedirs(help_dir)
        
    for filename in os.listdir(docs_dir):
        if filename.endswith('.md'):
            base = os.path.splitext(filename)[0]
            src = os.path.join(docs_dir, filename)
            dst = os.path.join(help_dir, base + '.txt')
            
            with open(src, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                
            plain_text = strip_markdown(content)
            
            with open(dst, 'w', encoding='utf-8') as f:
                f.write(plain_text)
                
            print(f"Synced and cleaned {filename} to {base}.txt")

if __name__ == '__main__':
    sync_docs_to_help()
