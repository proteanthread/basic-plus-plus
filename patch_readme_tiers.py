import re

with open('README.md', 'r', encoding='utf-8') as f:
    text = f.read()

# Fix mojibake
text = text.replace('?"', '—')
text = text.replace('^\'', '−')
text = text.replace('320A-200', '320×200')

# Fix Extensibility line
text = re.sub(r'Extensibility:\*\* DIALECT, ALIAS, MODULE, OPTION STRICT, COMPILE', 
              'Extensibility:** ALIAS, MODULE, COMPILE', text)

# We want to add Section 1: Interpreter Editions.
new_section = """## Section 1: Interpreter Editions

BASIC++ is distributed in three distinct compilation tiers, allowing it to scale from modern graphical workstations down to resource-constrained microcontrollers. The core architectural keywords, parsing logic, and execution semantics remain identical across all tiers; they differ only in their hardware abstractions and subsystem inclusions.

### 1.1. BASIC++ SDL (GUI Edition)
**Binaries:** asicpp.exe (Windows), aspp (Linux)
The full-featured graphical interpreter. This build statically links against SDL2, providing a dedicated GUI window upon boot. It supports the complete suite of visual and auditory features, including the 320x200 16-color virtual framebuffer (SCREEN 1), dynamic palette manipulation (COLOR, PALETTE), vector drawing (LINE, CIRCLE, PAINT), and audio synthesis (SOUND, PLAY).

### 1.2. BASIC++ Standard (Console Edition)
**Binaries:** asicpp-console.exe (Windows), aspp-console (Linux)
The standard command-line interface (CLI) interpreter. It boots directly into the host OS terminal (e.g., PowerShell, bash) without initializing a GUI window, ensuring seamless integration with shell pipes, redirections, and headless execution. However, it retains full compatibility with the SDL tier: if a program executes a graphical or audio command (like SCREEN 1), the interpreter will dynamically boot the SDL2 engine on-demand, create a window, and seamlessly continue execution.

### 1.3. BASIC++ Lite (Embedded Edition)
**Binaries:** lite.exe (Windows), lite (Linux)
A stripped-down, ultra-lightweight build optimized strictly for minimal memory footprints and execution speed. Designed for embedded environments (e.g., Arduino, Raspberry Pi Pico, FreeRTOS), it entirely omits the SDL layer, the graphics framebuffer, network subsystems, and other heavy external modules. It executes core BASIC logic and standard console I/O using standard C library functions.

---

"""

# Increment all section numbers
def increment_section(match):
    prefix = match.group(1)
    num = int(match.group(2)) + 1
    suffix = match.group(3)
    return f"{prefix}{num}{suffix}"

text = re.sub(r'(## Section )(\d+)(:)', increment_section, text)
text = re.sub(r'(### )(\d+)(\.)', increment_section, text)

# Now insert the new section 1 before what is now Section 2
text = text.replace('## Section 2: Core Features', new_section + '## Section 2: Core Features')

# Fix Example Session header
text = text.replace('BASIC++ 4.4.4', 'BASIC++ Standard 5.0.0')
text = text.replace('Jul 01 2026', 'Jul 10 2026')

with open('README.md', 'w', encoding='utf-8') as f:
    f.write(text)

print("Updated README.md")
