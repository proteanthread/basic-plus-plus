import re

with open('README.md', 'r', encoding='utf-8') as f:
    text = f.read()

# First, remove the badly placed section I just added
bad_section = """### 6.2. Deferred Mode (Edlin)
  
  While BASIC++ defaults to an Immediate Mode REPL, it also natively supports a Deferred Mode workflow (similar to 
QBASIC or modern IDEs) via the built-in screen editor. In Deferred Mode, statements are not evaluated or executed 
line-by-line as they are typed. Instead, you write your entire program within a text buffer offline. Once the program 
is fully written, it is passed to the host engine in a single batch for execution.
  
  To invoke Deferred Mode, launch the interpreter with the --edlin (or --edit) flag:
  
  > basicpp-console.exe --edlin
  
  Once inside the editor, you may use the x command to execute the entire buffer as a BASIC++ program.
"""

# Try to remove it using a regex because formatting might differ slightly
text = re.sub(r'### 6.2. Deferred Mode \(Edlin\).*?buffer as a BASIC\+\+ program\.\s*', '', text, flags=re.DOTALL)

# Now, add it as 6.3 properly
proper_section = """### 6.3. Deferred Mode (Edlin)

While BASIC++ defaults to an Immediate Mode REPL (Direct Mode), it also natively supports a Deferred Mode workflow (similar to QBASIC or modern IDEs) via the built-in screen editor. In Deferred Mode, statements are not evaluated or executed line-by-line as they are typed. Instead, you write your entire program within a text buffer offline. Once the program is fully written, it is passed to the host engine in a single batch for execution.

To invoke Deferred Mode, launch the interpreter with the --edlin (or --edit) flag:
`
> basicpp-console.exe --edlin
`
Once inside the editor, you may use the x command to execute the entire buffer as a BASIC++ program.

"""

text = text.replace('### 6.2. Program Mode', '### 6.2. Program Mode') # Keep as is
text = re.sub(r'(### 6.2. Program Mode.*?)(?=\n\s*In the event a BASIC program enters a non-terminating loop)', r'\1\n' + proper_section, text, flags=re.DOTALL)

with open('README.md', 'w', encoding='utf-8') as f:
    f.write(text)
