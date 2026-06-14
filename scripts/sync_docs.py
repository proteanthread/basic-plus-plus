import os
import shutil

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
            # For a real implementation, convert markdown to plain text
            # Currently just copying to keep them in sync
            shutil.copy2(src, dst)
            print(f"Synced {filename} to {base}.txt")

if __name__ == '__main__':
    sync_docs_to_help()
