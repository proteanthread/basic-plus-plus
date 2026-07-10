import re

with open('README.md', 'r', encoding='utf-8') as f:
    text = f.read()

# 1. "fork this into your own dialect" -> "fork this into your own implementation"
text = text.replace('fork this into your own dialect', 'fork this into your own implementation')

# 2. "BASIC++ ships with a runtime dialect-switching engine, a configurable 6-level security sandbox"
text = text.replace('BASIC++ ships with a runtime dialect-switching engine, a configurable 6-level security sandbox', 
                    'BASIC++ ships with a configurable 6-level security sandbox')

# 3. "The interpreter provides a comprehensive implementation of BASIC with support for 16 distinct dialect profiles, runtime dialect switching, and a union-mode parser that accepts the combined keyword set of all supported dialects by default."
text = re.sub(r'The interpreter provides a comprehensive implementation of BASIC with support for 16 distinct dialect profiles, runtime dialect switching, and a union-mode parser that accepts the combined keyword set of all supported dialects by default\.', 
              'The interpreter provides a comprehensive implementation of BASIC with a union-mode parser that accepts a comprehensive keyword set covering standard BASIC features by default.', text)

# 4. "Extensibility:** DIALECT, ALIAS, MODULE, OPTION STRICT, COMPILE"
text = text.replace('Extensibility:** DIALECT, ALIAS, MODULE, OPTION STRICT, COMPILE', 
                    'Extensibility:** ALIAS, MODULE, COMPILE')

# 5. "Platform/dialect-independent bytecode format" -> "Platform-independent bytecode format"
text = text.replace('Platform/dialect-independent bytecode format', 'Platform-independent bytecode format')

# 6. Remove Section 2 entirely
text = re.sub(r'## Section 2: Multi-Dialect Engine.*?## Section 3: Security Sandboxing', 
              '## Section 2: Security Sandboxing', text, flags=re.DOTALL)

# Re-number remaining sections
text = text.replace('## Section 4: Architecture', '## Section 3: Architecture')
text = text.replace('## Section 5: Compilation', '## Section 4: Compilation')
text = text.replace('## Section 6: Operational Use', '## Section 5: Operational Use')
text = text.replace('### 6.1. Direct Mode', '### 5.1. Direct Mode')
text = text.replace('### 6.2. Program Mode', '### 5.2. Program Mode')
text = text.replace('### 6.3. Deferred Mode (Edlin)', '### 5.3. Deferred Mode (Edlin)')
text = text.replace('### 6.4. Program Execution', '### 5.4. Program Execution')

text = text.replace('## Section 7: Halting Non-Terminating Execution', '## Section 6: Halting Non-Terminating Execution')
text = text.replace('## Section 8: Virtual Device Layer', '## Section 7: Virtual Device Layer')
text = text.replace('### 8.1. Built-In Devices', '### 7.1. Built-In Devices')
text = text.replace('### 8.2. Virtual Memory Maps', '### 7.2. Virtual Memory Maps')
text = text.replace('### 8.3. Graphics Framebuffer', '### 7.3. Graphics Framebuffer')

text = text.replace('## Section 9: Module System', '## Section 8: Module System')
text = text.replace('### 9.1. Built-In Modules', '### 8.1. Built-In Modules')
text = text.replace('### 9.2. Creating External Modules', '### 8.2. Creating External Modules')

text = text.replace('## Section 10: Transpiler', '## Section 9: Transpiler')
text = text.replace('## Section 11: Future Expansion Trajectory', '## Section 10: Future Expansion Trajectory')
text = text.replace('### 11.1.', '### 10.1.')
text = text.replace('### 11.2.', '### 10.2.')
text = text.replace('### 11.3.', '### 10.3.')
text = text.replace('### 11.4.', '### 10.4.')

text = text.replace('## Section 12: Documentation', '## Section 11: Documentation')
text = text.replace('## Section 13: Example Session', '## Section 12: Example Session')
text = text.replace('## Section 14: Project Statistics', '## Section 13: Project Statistics')

# 7. Remove dialect.c/h
text = re.sub(r'\|\s*dialect\.c/h\s*\|\s*12 dialect profiles, strict mode, feature gating\s*\|\n?', '', text)

# 8. Remove DIALECT "QBAS" : PRINT "Now in QBasic mode" in example session
text = re.sub(r'> DIALECT "QBAS" : PRINT "Now in QBasic mode"\nNow in QBasic mode\n?', '', text)

# 9. Remove 6. **Dialect Configuration** — Queries current dialect personality properties.
text = re.sub(r'6\. \*\*Dialect Configuration\*\*\s*(?:â€”|—|-)\s*Queries current dialect personality properties\.\n?', '', text)
# Need to re-number self tests
text = text.replace('7. **Parser Precedence**', '6. **Parser Precedence**')
text = text.replace('8. **Loop Control Flow**', '7. **Loop Control Flow**')
text = text.replace('9. **VFS File I/O**', '8. **VFS File I/O**')
text = text.replace('10. **Device Aliases**', '9. **Device Aliases**')
text = text.replace('11. **Graphics/SDL2**', '10. **Graphics/SDL2**')

# 10. Fix module explanation
text = text.replace('enabling the creation of dialect-specific feature sets', 'enabling the creation of specific feature sets')

# 11. Remove documentation lines
text = re.sub(r'\|\s*Mixing_Dialects\.txt\s*\|\s*Multi-dialect programming\s*\|\n?', '', text)
text = re.sub(r'\|\s*Older_Dialects\.txt\s*\|\s*Emulating classic systems and memory maps\s*\|\n?', '', text)
text = re.sub(r'\|\s*Creating_Dialects\.txt\s*\|\s*Building custom dialect configurations\s*\|\n?', '', text)

# 12. Remove Dialect profiles row
text = re.sub(r'\|\s*Dialect profiles\s*\|\s*\d+\s*\|\n?', '', text)

# 13. Fix transcendental function description "activated via numeric literals containing a decimal point (e.g., 3.14) or via dialect configuration."
text = text.replace('activated via numeric literals containing a decimal point (e.g., 3.14) or via dialect configuration.', 'activated via numeric literals containing a decimal point (e.g., 3.14).')

with open('README.md', 'w', encoding='utf-8') as f:
    f.write(text)

print("Updated README.md")
