import re

with open('README.md', 'r', encoding='utf-8') as f:
    text = f.read()

new_section = """
### 6.2. Deferred Mode (Edlin)

While BASIC++ defaults to an Immediate Mode REPL, it also natively supports a Deferred Mode workflow (similar to QBASIC or modern IDEs) via the built-in screen editor. In Deferred Mode, statements are not evaluated or executed line-by-line as they are typed. Instead, you write your entire program within a text buffer offline. Once the program is fully written, it is passed to the host engine in a single batch for execution.

To invoke Deferred Mode, launch the interpreter with the --edlin (or --edit) flag:
`
> basicpp-console.exe --edlin
`
Once inside the editor, you may use the x command to execute the entire buffer as a BASIC++ program.
"""

text = text.replace('### 6.2. Deferred Mode', new_section)
if new_section not in text:
    text = text.replace('### 6.1. Direct Mode', '### 6.1. Direct Mode\n' + new_section)

with open('README.md', 'w', encoding='utf-8') as f:
    f.write(text)
