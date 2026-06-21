[![GitGem](https://gitgem.org/api/badge/github/proteanthread/basic-plus-plus.svg)](https://gitgem.org/github/proteanthread/basic-plus-plus)

# BASIC++ (protoBASIC) Interpreter
**Version 4.0.1 (Stable)**

I don't care what you do with my code, just don't take my code and sell it and/or don't take my code, modify my code, and sell it. This code is not for sale.

### I do encourage you to fork this into your own dialect.  Everything you need is right there in the source.

---

## Abstract

A portable, multi-dialect BASIC interpreter written in 115 C17 source files and 60 header files (~90,000 lines of code). Organized into 28 domain subdirectories with a modular, layered architecture. Zero external dependencies -- compiles on any platform with a standards-compliant C compiler.

BASIC++ ships with 16 historically accurate dialect profiles, a runtime dialect-switching engine, a configurable 6-level security sandbox, a virtual device layer, a plugin system, a native code transpiler, and 70 documentation files.

Designed for small memory footprints and readable source code. Runs on Windows 11, Linux, and FreeDOS. Suitable for embedded systems, legacy hardware, and as a teaching tool for interpreter design (tokenization, recursive-descent parsing, virtual machines, and environment management).


---


## Section 1: Core Features

The interpreter provides a comprehensive implementation of BASIC with support for 16 distinct dialect profiles, runtime dialect switching, and a union-mode parser that accepts the combined keyword set of all supported dialects by default.

### 1.1. Data Types

BASIC++ supports three fundamental data types:

- **Integers** — 32-bit signed (`long`), providing a range of −2,147,483,648 to +2,147,483,647. All integer division is truncating (e.g., `7 / 3` evaluates to `2`).
- **Floating-point** — Double-precision IEEE 754 (`double`), activated via numeric literals containing a decimal point (e.g., `3.14`) or via dialect configuration. Supports the full suite of transcendental functions: `SIN`, `COS`, `TAN`, `ATN`, `SQR`, `LOG`, `EXP`.
- **Strings** — Variable-length character sequences up to 255 characters, managed via a pooled allocator. String variables are denoted by the `$` suffix (e.g., `A$`, `NAME$`).

### 1.2. Variable Storage

The variable system provides three tiers of storage:

| Tier | Capacity | Scope | Description |
|:-----|:---------|:------|:------------|
| Single-letter | 26 numeric (`A`–`Z`), 26 string (`A$`–`Z$`) | Global | Direct array-index lookup, zero overhead |
| Named variables | Up to 256 identifiers, 31 characters max | Global | Hash-based lookup (e.g., `SCORE`, `PLAYER_NAME$`) |
| DIM arrays | Up to 64 arrays, 2 dimensions max, 8,192 total elements | Global | Row-major flat pool, supports numeric and string arrays |

All variables are initialized to zero (numeric) or empty string (string) upon `RUN`. The `CLEAR` command resets all variable storage without affecting the stored program.

### 1.3. Parser

Expression evaluation is conducted via a recursive-descent parser with correct mathematical operator precedence:

```
^                     (highest — exponentiation)
- (unary), NOT        (negation)
*, /, \               (multiply, divide, integer-divide)
MOD                   (modulo)
+, -                  (add, subtract, string concatenation)
=, <>, <, >, <=, >=   (comparison)
AND                   (bitwise/logical AND)
OR, XOR               (bitwise/logical OR, exclusive OR)
EQV, IMP              (equivalence, implication — lowest)
```

Sub-expressions encapsulated in parentheses are evaluated recursively, permitting explicit enforcement of evaluation order. The parser supports both line-numbered program mode and unnumbered direct (immediate) mode execution.

### 1.4. Implemented Directives

The interpreter implements over 220 keywords spanning the following categories:

**Data I/O:** `PRINT`, `PRINT USING`, `LPRINT`, `INPUT`, `LINE INPUT`, `READ`, `DATA`, `RESTORE`, `WRITE`

**Assignment:** `LET` (optional), `SWAP`, `CONST`

**Program Flow:** `GOTO`, `GOSUB`, `RETURN`, `IF...THEN...ELSE`, `FOR...NEXT`, `WHILE...WEND`, `DO...LOOP`, `SELECT CASE`, `ON...GOTO`, `ON...GOSUB`, `EXIT`

**Subroutines & Functions:** `SUB...END SUB`, `FUNCTION...END FUNCTION`, `CALL`, `DEF FN`, `SHARED`, `STATIC`

**Arrays & Matrices:** `DIM`, `REDIM`, `ERASE`, `OPTION BASE`, `MAT READ`, `MAT PRINT`, `MAT` arithmetic (`+`, `-`, `*`), `MAT ZER`, `MAT CON`, `MAT IDN`, `MAT TRN`, `MAT INV`

**File I/O:** `OPEN`, `CLOSE`, `INPUT #`, `PRINT #`, `LINE INPUT #`, `WRITE #`, `GET`, `PUT`, `SEEK`, `LOF`, `LOC`, `EOF`, `FIELD`, `LSET`, `RSET`

**Error Handling:** `ON ERROR GOTO`, `RESUME`, `RESUME NEXT`, `ERR`, `ERL`, `ERROR`

**String Functions:** `LEN`, `LEFT$`, `RIGHT$`, `MID$`, `ASC`, `CHR$`, `VAL`, `STR$`, `INSTR`, `LCASE$`, `UCASE$`, `LTRIM$`, `RTRIM$`, `SPACE$`, `STRING$`, `HEX$`, `OCT$`

**Graphics & Sound:** `SCREEN`, `PSET`, `PRESET`, `LINE`, `CIRCLE`, `DRAW`, `PAINT`, `PALETTE`, `COLOR`, `SOUND`, `PLAY`, `BEEP`

**Screen & Console:** `CLS`, `LOCATE`, `WIDTH`, `CSRLIN`, `POS`, `INKEY$`, `KEY`

**User-Defined Types:** `TYPE...END TYPE`, typed variable fields, arrays of records

**System & Shell:** `SHELL`, `SHELL$()`, `EXEC`, `ENVIRON`, `ENVIRON$()`, `CHDIR`, `MKDIR`, `RMDIR`, `KILL`, `NAME`, `FILES`

**Memory:** `PEEK`, `POKE`, `DEF SEG`, `VARPTR`, `FRE`

**Environment:** `NEW`, `RUN`, `LIST`, `SAVE`, `LOAD`, `MERGE`, `CHAIN`, `RENUM`, `DELETE`, `AUTO`, `EDIT`, `TRON`, `TROFF`, `STOP`, `CONT`, `BREAK`, `VARS`, `VER`, `HELP`, `INFO`, `CATALOG`, `DIR`, `BYE`

**Security:** `SECURITY LEVEL`, `SECURITY REPORT`

**Extensibility:** `DIALECT`, `ALIAS`, `MODULE`, `OPTION STRICT`, `COMPILE`

### 1.5. Environment Directives

A distinct set of directives, which operate at the "edit" level outside stored programs, are provided for managing the runtime environment:

| Command | Function |
|:--------|:---------|
| `RUN [line]` | Execute program (optionally from a specific line) |
| `LIST [n1[-n2]]` | Display stored program lines |
| `NEW` | Clear program memory and variables |
| `SAVE "filename"` | Persist program to disk |
| `LOAD "filename"` | Retrieve program from disk |
| `MERGE "filename"` | Merge file into current program |
| `RENUM [start [, step]]` | Renumber program lines |
| `DELETE n1-n2` | Delete a range of program lines |
| `AUTO [start [, step]]` | Automatic line numbering mode |
| `EDIT line` | Edit a specific program line |
| `SELFTEST` | Run built-in diagnostic test suite |
| `VER` | Display version, copyright, and build date |
| `HELP [keyword]` | Display help for a command or topic |
| `BYE` | Exit the interpreter to the OS prompt |

### 1.6. Input/Output Operations

The core implementation provides multiple output pathways:

- **`PRINT`** — Output to the primary console (standard output) with support for string literals, numeric expressions, format specifiers (`,` zone-based, `;` packed), and `PRINT USING` for formatted output with template strings.
- **`LPRINT`** — Redirects output to the error device (`lprint.out`), simulating a physical line printer.
- **File I/O** — Full GW-BASIC/QBasic-compatible file operations supporting sequential (`INPUT`, `OUTPUT`, `APPEND`), random-access (`RANDOM`), and binary (`BINARY`) modes across 8 simultaneous file channels (`#1` through `#8`).
- **Shell integration** — `SHELL "command"` for synchronous execution, `SHELL$("command")` for output capture, pipe (`|`) and redirect (`>`, `>>`) operators.


---


## Section 2: Multi-Dialect Engine

BASIC++ is unique in its ability to emulate 12 historically accurate BASIC dialects within a single interpreter. Each dialect profile configures statement separators, operator behavior, ready prompts, print zone widths, feature gates, and keyword availability.

### 2.1. Supported Dialects

| Code | Dialect | Year | Prompt | Separator | Notes |
|:-----|:--------|:-----|:-------|:----------|:------|
| `PATB` | Palo Alto Tiny BASIC | 1976 | `READY` | `;` | Li-Chen Wang. Integer-only, `@()` arrays, `#` for `<>` |
| `TRS1` | TRS-80 Level I | 1977 | `READY` | `:` | Leininger. Integer-only, 26 vars, basic strings |
| `TRS2` | TRS-80 Level II | 1979 | `READY` | `:` | Microsoft. Full float, string arrays, multi-dim |
| `GWBS` | GW-BASIC | 1983 | `Ok` | `:` | Microsoft. IBM PC workhorse, WHILE/WEND, ON ERROR |
| `EC55` | ECMA-55 Minimal BASIC | 1978 | `READY` | `:` | ISO standard. Requires LET, formal specification |
| `E116` | ECMA-116 Full BASIC | 1986 | `READY` | `:` | ISO standard. Structured flow, matrices, exceptions |
| `QBAS` | QBasic | 1991 | `Ok` | `:` | Microsoft. SUB/FUNCTION, SELECT CASE, long names |
| `AINT` | Apple II Integer BASIC | 1977 | `>` | `:` | Wozniak. Integer-only, no float, limited strings |
| `ASFT` | AppleSoft BASIC | 1977 | `]` | `:` | Microsoft for Apple II. Full float, standard MS |
| `ATRI` | Atari BASIC | 1979 | `READY` | `:` | Shepardson. Tokenized storage, CLR, DIM strings |
| `C64B` | Commodore BASIC v2 | 1982 | `READY.` | `:` | Microsoft 6502. PEEK/POKE/SYS, limited error handling |
| `COCO` | Color Computer BASIC | 1980 | `OK` | `:` | Microsoft Extended Color BASIC for Tandy CoCo |

### 2.2. Dialect Switching

Dialects can be switched at any time during a session:

```basic
DIALECT "QBAS"        ' Switch to QBasic mode
DIALECT "C64B"        ' Switch to Commodore 64 mode
DIALECT LIST          ' List all available dialects
```

### 2.3. Strict Mode

By default, BASIC++ operates in **union mode**, where all keywords from all dialects are accepted. Enabling strict mode restricts the parser to only the keywords that belong to the active dialect's historical feature set:

```basic
OPTION STRICT         ' Enable dialect-strict parsing
OPTION STRICT OFF     ' Return to union mode
```

### 2.4. Keyword Aliasing

The `ALIAS` system allows keyword remapping for localization or personal preference:

```basic
ALIAS "IMPRIME" = PRINT       ' Spanish alias for PRINT
ALIAS "ESCRIBE" = WRITE       ' Spanish alias for WRITE
ALIAS LIST                    ' Show active aliases
ALIAS CLEAR ALL               ' Remove all aliases
```


---


## Section 3: Security Sandboxing

BASIC++ includes a three-tier security model that controls access to sensitive operations. This is critical for environments where untrusted BASIC programs may be executed (e.g., BBS systems, educational labs, online services).

### 3.1. Security Levels

| Level | Name | File Read | File Write | Shell | Network |
|:------|:-----|:----------|:-----------|:------|:--------|
| 0 | `OPEN` | ✅ | ✅ | ✅ | ✅ |
| 1 | `STANDARD` | ✅ | ✅ | ❌ | ❌ |
| 2 | `RESTRICTED` | ❌ | ❌ | ❌ | ❌ |

```basic
SECURITY LEVEL 2      ' Lock down to restricted mode
SECURITY REPORT       ' Display current security posture
```


---


## Section 4: Architecture

### 4.1. Source File Organization

The interpreter is organized into 29 compilation units:

| File | Purpose |
|:-----|:--------|
| `main.c` | Boot sequence, REPL loop, shutdown |
| `lexer.c/h` | Tokenizer with 223-keyword table, alias support |
| `parser.c/h` | Recursive-descent parser with direct execution dispatch |
| `runtime.c/h` | Runtime state, call stack, FOR/NEXT frames |
| `memory.c/h` | Pool allocator, program store, scratch buffer |
| `value.c/h` | Tagged union value system (int, float, string) |
| `stringpool.c/h` | Compact string allocator with GC-safe pooling |
| `dialect.c/h` | 12 dialect profiles, strict mode, feature gating |
| `errors.c/h` | Error codes, BASIC-style error messages |
| `fileio.c/h` | Sequential, random-access, and binary file I/O |
| `vdev.c/h` | Virtual device layer (console, error, file, user) |
| `vm.c/h` | Virtual machine formalization, opcode dispatch table |
| `funcreg.c/h` | Function registry with override support |
| `builtins.c/h` | Built-in math and string function implementations |
| `detok.c/h` | Detokenizer for LIST, diagnostics, and debugging |
| `exec.c/h` | Program execution engine (RUN loop) |
| `ast.c/h` | Abstract syntax tree node types |
| `codegen.c/h` | C code generator for the transpiler |
| `compiler.c/h` | BASIC-to-C transpiler driver |
| `security.c/h` | Security level enforcement and operation gating |
| `module.c/h` | Module registration and lifecycle |
| `mod_stdlib.c/h` | Standard library module (built-in) |
| `mod_usb.c/h` | USB HID and serial device module |
| `help.c/h` | Interactive help system |
| `selftest.c/h` | Built-in self-test suite |
| `memmap.c/h` | Virtual memory maps (MSDOS, C64, Atari, Apple, ZX) |
| `platform.c/h` | Platform detection and OS abstraction |
| `gfxbuf.c/h` | Graphics framebuffer (320×200, 16 colors) |
| `bytecode.c/h` | Bytecode compilation infrastructure |
| `config.h` | All compile-time constants and limits |

### 4.2. Memory Layout

All interpreter memory is defined by static, fixed-size pools. Dimensions are established at compile-time via `#define` constants in `config.h`, ensuring a predictable and verifiable memory footprint.

| Memory Area | Constant | Default Size | Description |
|:------------|:---------|:-------------|:------------|
| Program Storage | `MAX_PROGRAM_LINES` | 4,096 lines | Stored BASIC program lines |
| Program Pool | `PROGRAM_MEMORY_SIZE` | 64 KB | Raw program text storage |
| Variable Pool | `VARIABLE_MEMORY_SIZE` | 64 KB | Variable and array storage |
| Scratch Pool | `SCRATCH_MEMORY_SIZE` | 64 KB | Temporary token/expression buffers |
| String Pool | `MAX_STRING_POOL` | 32 KB | Runtime string allocations |
| Array Elements | `MAX_ARRAY_ELEMENTS` | 8,192 | Flat pool shared across all arrays |
| Virtual Memory | `MAX_MEM_SEGMENT` | 64 KB | PEEK/POKE address space |
| Call Stack | `MAX_STACK_DEPTH` | 256 levels | GOSUB, FOR/NEXT, SUB/FUNCTION frames |
| File Channels | `MAX_FILE_CHANNELS` | 8 | Simultaneous open files |

### 4.3. Adjustment of Memory Allocations

Alterations to these memory limitations are effectuated by modifying the appropriate `#define` pre-processor constants within `config.h`. Subsequent recompilation of the interpreter is mandatory for such changes to take effect. This compile-time configuration is a deliberate design choice, precluding runtime memory negotiation. This approach ensures that the interpreter's resource requirements are fixed and verifiable, a critical attribute for high-reliability systems, embedded applications, or legacy operating systems where dynamic memory management is complex or unreliable.


---


## Section 5: Compilation

The C source code is designed for high portability and is compilable on any system featuring a standards-compliant C compiler. No external libraries, package managers, or build frameworks are required.

### 5.1. Quick Build

```bash
# Windows (MSVC — from Developer Command Prompt)
cl /TC /W3 /O2 /Fe:basicpp.exe *.c

# Linux / macOS (GCC or Clang)
gcc -O2 -o basicpp *.c -lm
clang -O2 -o basicpp *.c -lm

# FreeDOS (OpenWatcom)
wcc -ml -0 -za -wx *.c
wlink name basicpp.exe file *.obj
```

### 5.2. Compilation for Portability and Size *(Recommended)*

```bash
gcc -Wall -Os -o basicpp *.c -lm
```

This incantation invokes the compiler with `-Wall` to enable all high-priority warnings, a best practice for identifying potential portability issues or unsafe code. Crucially, it uses `-Os`, which instructs the compiler to optimize specifically for the size of the resulting executable binary. This optimization level is often the primary concern in memory-constrained systems, such as the target embedded environments.

### 5.3. Compilation for Execution Speed

```bash
gcc -Wall -O2 -o basicpp *.c -lm
```

This command uses the `-O2` flag, enabling a more aggressive set of optimization passes (such as loop unrolling and function inlining) focused on increasing execution velocity. This may come at the cost of a slightly larger binary file. This build is suitable for desktop systems where performance is prioritized over footprint.

### 5.4. Compilation for Debugging *(Symbolic Inclusion)*

```bash
gcc -Wall -g -O0 -DDEBUG -o basicpp *.c -lm
```

This command utilizes the `-g` flag to include debugging symbols (such as DWARF) within the final executable. The `-DDEBUG` flag enables debug-mode assertions and verbose diagnostics within the interpreter. This symbolic information is essential for using a debugger (such as GDB) to trace program execution, inspect variables, and analyze the call stack.

### 5.5. Using the Makefile

A `Makefile` is provided for incremental builds:

```bash
make              # GCC release build (default)
make debug        # GCC debug build with symbols
make msvc         # MSVC build (from VS command prompt)
make watcom       # OpenWatcom build (FreeDOS)
make clean        # Remove build artifacts
```


---


## Section 6: Operational Use

The interpreter operates via a standard REPL (Read-Evaluate-Print Loop) interface. This interface provides two distinct contexts for operation: Direct Mode and Program Mode.

### 6.1. Direct Mode

The direct, or "immediate," execution context is invoked when directives are entered without a preceding line number. Such directives are evaluated and executed immediately upon entry. This mode is principally utilized for testing, debugging, performing calculations, or inspecting variable state.

```
> PRINT 10 + 5
     15
> A = 42 : PRINT A * 2
     84
> DIALECT "QBAS" : PRINT "Now in QBasic mode"
Now in QBasic mode
```

### 6.2. Program Mode

The "stored program" context is invoked when directives are entered with a preceding line number. Such lines are not executed; instead, they are inserted into the Program Storage array, maintained in sorted order by line number.

```
> 10 PRINT "Hello, World!"
> 20 FOR I = 1 TO 5
> 30 PRINT I; " ";
> 40 NEXT I
> 50 END
> LIST
10 PRINT "Hello, World!"
20 FOR I = 1 TO 5
30 PRINT I; " ";
40 NEXT I
50 END
> RUN
Hello, World!
 1  2  3  4  5
```

### 6.3. Program Execution

The `RUN` directive initiates sequential execution of the stored program. This directive first clears Variable Storage and the Call Stack to a zeroed state, ensuring that the program executes in a clean, predictable environment. Execution begins at the lowest extant line number. The `BYE` command exits the interpreter entirely, returning control to the operating system.


---


## Section 7: Halting Non-Terminating Execution

In the event a BASIC program enters a non-terminating loop, which is a common possibility given the `GOTO` directive, execution may be interrupted by two mechanisms:

1. **`Ctrl+C` (SIGINT)** — Issuing an interrupt signal from the controlling terminal. The host operating system will halt the interpreter process and return control to the command shell.
2. **`STOP` / `BREAK`** — If placed within a program, the `STOP` statement suspends execution and enters direct mode, allowing inspection of variables. Execution may be resumed with `CONT`. The `BREAK` command sets breakpoints for the interactive debugger.


---


## Section 8: Virtual Device Layer

BASIC++ abstracts all I/O through a virtual device (`VDev`) interface, enabling portability across operating systems and hardware configurations without modifying the core interpreter.

### 8.1. Built-In Devices

| Device | ID | Function |
|:-------|:---|:---------|
| Console | `dev_con` | Primary screen output and keyboard input |
| Error | `dev_err` | Error/diagnostic output (stderr) |
| File | `dev_file` | File I/O channels (#1 through #8) |
| Printer | `dev_lpt` | LPRINT output device |

### 8.2. Virtual Memory Maps

The `MEMMAP` system provides pre-configured virtual address spaces that emulate classic platforms:

```basic
MEMMAP "C64"          ' Commodore 64 memory layout
MEMMAP "MSDOS"        ' IBM PC / MS-DOS layout
MEMMAP "APPLE2"       ' Apple II memory map
MEMMAP "ATARI8"       ' Atari 400/800 layout
MEMMAP "ZX"           ' ZX Spectrum layout
```

### 8.3. Graphics Framebuffer

A 320×200, 16-color virtual framebuffer is provided, matching QBasic `SCREEN 1`. Graphics are rendered to the terminal using Unicode half-block characters.


---


## Section 9: Module System

The module system provides C-level code extensibility. Modules add new keywords, functions, and hardware abstractions to the interpreter.

### 9.1. Built-In Modules

| Module | Description |
|:-------|:------------|
| `STDLIB` | Standard library — core mathematical and string functions |
| `USB` | USB HID (gamepads, joysticks) and USB serial (FTDI, CH340, Arduino) |

### 9.2. Creating External Modules

External modules follow the `ModuleInterface` contract: an `init()`, `shutdown()`, and keyword registration via `funcreg_override()`. See `External_Modules.txt` for the complete API specification.


---


## Section 10: Transpiler

BASIC++ includes a BASIC-to-C transpiler that converts stored programs into standalone C source code, compilable into native executables with no runtime dependency on the interpreter.

```basic
10 PRINT "Hello, World!"
20 END
COMPILE "hello"       ' Generates hello.c
```

The generated C code is self-contained ANSI C89, suitable for compilation on any target platform.


---


## Section 11: Future Expansion Trajectory

The architecture is explicitly provisioned for future expansion. The following classifications for extensibility have been identified:

### 11.1. Addons

This classification is designated for the most advanced extensibility, involving the incorporation of inline foreign language code. This system would provide meta-directives (e.g., `$LANG: C`) to allow a user to embed, compile, and link source code from other languages, such as Assembly, Pascal, or C, directly within a BASIC program file. This represents the ultimate goal of a mixed-language development environment, likely implemented via a transpiler and external compiler-chaining.

### 11.2. Merge

This classification refers to functionality for BASIC source code amalgamation, specifically the `MERGE` directive. This system maintains strict compliance with the behavioral standards of ECMA-55 (Minimal BASIC), ECMA-116 (Full BASIC), and/or QBasic/QuickBASIC. Its function is to load a BASIC program file from storage and combine it with the program already resident in memory, with lines from the incoming file overwriting any pre-existing lines with identical numbers. This is the foundational pillar for user-level code sharing.

### 11.3. Modules

This classification defines the primary system for C-level code extensibility. A "Module" is a compiled C-code entity that adds new keywords and syntactic features to the interpreter. This system is responsible for language syntax modification, enabling the creation of dialect-specific feature sets (e.g., adding a GRAPHICS module to provide `PSET` and `LINE`, or a SOUND module to provide `PLAY`). This is the mechanism by which the interpreter evolves from "Core" to "Full" BASIC.

### 11.4. Plugins

This classification defines a specialized subset of Modules. A "Plugin" is a C-code module designated for low-level hardware mapping, system emulation, and direct memory interfacing. A Plugin functions as a "driver," abstracting the hardware. For example, a SOUND Module (Section 11.3) provides the `SOUND` keyword, but it calls a SOUND Plugin (e.g., `pc_speaker.plugin` for DOS or `oss.plugin` for Linux) to actually generate the audio. This architectural separation of semantics (Module) from implementation (Plugin) is the key to achieving cross-platform portability for hardware-dependent features.


---


## Section 12: Documentation

A comprehensive documentation suite of 36 reference manuals and tutorials is included:

| Document | Subject |
|:---------|:--------|
| `Users_Guide.txt` | Getting started, environment, commands |
| `Programmers_Guide.txt` | Complete language reference |
| `How_To_Compile.txt` | Building from source on all platforms |
| `Quick_Reference.txt` | Alphabetical keyword reference card |
| `Self_Programming.txt` | Meta-programming and self-modification |
| `Scripting_Functions.txt` | Shell integration, pipes, redirects |
| `Mixing_Dialects.txt` | Multi-dialect programming |
| `Using_Aliases.txt` | Keyword remapping with ALIAS |
| `Arrays_And_Matrices.txt` | DIM, REDIM, MAT operations, sorting |
| `File_IO.txt` | Sequential, random-access, binary files |
| `Older_Dialects.txt` | Emulating classic systems and memory maps |
| `Creating_Dialects.txt` | Building custom dialect configurations |
| `Advanced_DEF.txt` | DEF FN, FUNCTION/SUB, closures |
| `External_Modules.txt` | Modules, plug-ins, system services |
| `Error_Handling.txt` | ON ERROR, RESUME, ERR, ERL |
| `Graphics_Sound.txt` | SCREEN, DRAW, LINE, SOUND, PLAY |
| `Debugging.txt` | TRON, TROFF, STOP, CONT, BREAK |
| `Security.txt` | Sandboxing and trust levels |
| `Secure_Coding.txt` | Writing safe and defensive BASIC++ code |
| `Virtual_Devices.txt` | VDev system, INP, OUT, custom devices |
| `Virtual_Machines.txt` | Virtual machines, consoles, terminals |
| `Virtual_Filesystem.txt` | Safe local file access |
| `Virtual_Network.txt` | TCP, UDP, TLS, Telnet, SSH, FTP, IRC, HTTP |
| `USB_Devices.txt` | USB HID and serial device support |
| `Compiling_BASIC_Programs.txt` | Transpile BASIC to native executables |
| `Memory_Maps.txt` | Creating and using MEMMAP presets |
| `Systems_Programming.txt` | Bare-metal execution, drivers, OS design |
| `User_Defined_Types.txt` | TYPE...END TYPE, records, typed fields |
| `Screen_And_Console.txt` | LOCATE, COLOR, CLS, WIDTH, PRINT USING |
| `Subroutines_And_Functions.txt` | GOSUB, SUB/FUNCTION, DEF FN, CALL |
| `String_Handling.txt` | String functions, pool architecture |
| `Internals_And_Architecture.txt` | Boot sequence, memory, parser pipeline |


---


## Section 13: Example Session

```
BASIC++ 4.0.1
@COPYLEFT ALL WRONGS RESERVED
Jun  8 2026

Ready.
> 10 INPUT "Your name"; N$
> 20 PRINT "Hello, "; N$; "!"
> 30 FOR I = 1 TO 3
> 40 PRINT I; " Mississippi..."
> 50 NEXT I
> 60 END
> RUN
Your name? World
Hello, World!
 1 Mississippi...
 2 Mississippi...
 3 Mississippi...
> SAVE "hello.bas"
> BYE
Goodbye.
```


---


## Section 14: Project Statistics

| Metric | Value |
|:-------|:------|
| Source files | 115 `.c` + 60 `.h` |
| Lines of code | ~90,000 |
| Keywords | 223 |
| Dialect profiles | 16 |
| Documentation files | 70 |
| External dependencies | **Zero** |
| C standard | C17 |
| License | @COPYLEFT ALL WRONGS RESERVED |


---

*BASIC++ — Because the world needed one more BASIC interpreter.*
