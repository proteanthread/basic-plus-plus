import re

with open('README.md', 'r', encoding='utf-8') as f:
    text = f.read()

text = text.replace('### 1.3. BASIC++ Lite (Embedded Edition)', '### 1.3. BASIC++ Lite3 (Embedded Edition)')

with open('README.md', 'w', encoding='utf-8') as f:
    f.write(text)
