# BASIC++ v6.5.2 Engine Features, Subsystems & Micro-Libraries Reference (`engine/Engine_Features.md`)

> **Authoritative Engine Source Reference**: Generated and maintained strictly from the `engine/` source code (`engine/src/` and `engine/include/`).

---

## 1. Product Architecture & 11-Modular Library Spectrum

The BASIC++ v6.5.2 engine is structured into an 11-modular library spectrum linked accumulatively (`libboot` -> `libplatform` -> `libkernel` -> `libengine` -> `libhardware` -> `libserver` -> `libscript` -> `libcore` -> `libflex` -> `libstandard` -> `libadvanced` -> `libext`), ensuring strict C17 portability, modularity, and zero hidden initialization dependencies.

| Library | Subsystem / Responsibility | Key Source Modules (`engine/src/`) |
|---|---|---|
| **`libboot`** | Bootloader sequence controller | `bootstrap/boot.c`, `bootstrap/common/common.c` |
| **`libplatform`** | OS platform abstraction (zero OS calls in upper layers) | `platform/plat_console.c`, `plat_fs.c`, `plat_sys.c`, `plat_time.c`, `plat_thread.c`, `plat_dl.c`, `plat_net.c`, `plat_regex.c`, `plat_clipboard.c` |
| **`libkernel`** | Core VM context, lexer, memory manager, security sandbox, BIOS virtualization, `VDev`/`VCon` device bus | `vm/context.c`, `lexer/lexer.c`, `memory/memory.c`, `security/security.c`, `bios/bios_pc.c`, `device/vdev.c`, `device/vcon.c` |
| **`libengine`** | AST evaluator, RPN stack engine, parser, runtime functions, variables, strings, bytecode looper | `eval/eval.c`, `eval/rpn.c`, `parser/parser.c`, `vm/exec.c`, `runtime/variables.c`, `runtime/strings.c` |
| **`libhardware`** | Segmented memory (`vmem`), BGI software rasterizer, FujiNet hardware emulation | `device/bgi/bgi_core.c`, `bgi_raster.c`, `bgi_modes.c`, `bgi_palette.c`, `bgi_font.c`, `device/fujinet.c` |
| **`libserver`** | Network sockets (`vnet`), Gemini protocol, background tasks, VFS, crypto, regex | `device/vnet.c`, `server/gemini.c`, `server/task.c`, `security/crypto.c` |
| **`libscript`** | Headless batch script runner and file I/O operations | `bootstrap/server/server.c`, `statements/filesystem/` |
| **`libcore`** | Foundational REPL environment, numeric formatting, PRINT USING engine, metadata registry | `bootstrap/iot/iot.c`, `runtime/numfmt.c`, `runtime/metadata.c` |
| **`libflex`** | Dynamic metaprogramming (`ALIAS`, `OVERRIDE`, `SCOPE`), module loader, math & array extensions | `scope/scope.c`, `module/module.c`, `eval/functions/` |
| **`libstandard`** | Standard TUI workstation, multi-window TUI editor multiplexer, DAP debug server | `editor/tui_multiplexer.c`, `editor/editor_buffer.c`, `debug/dap.c` |
| **`libadvanced`** | Desktop visual graphics (`baspp`), multimedia, SDL2/OpenGL bindings | `bootstrap/desktop/desktop.c`, `device/gfx.c` |
| **`libinterop_core`** | Cross-language handle table, COM interop, JSON-RPC, Universal IR | `interop/interop_core.c`, `interop_handle.c`, `interop_ipc.c`, `interop_com.c`, `compiler/compiler_ir.c` |

---

## 2. Executable Target Editions & Memory Allocations

BASIC++ builds into seven distinct executable binary targets optimized for different execution environments:

| Target Executable | Edition Name | Default Memory Pool | Interactive Prompt | Output Status | Graphic/TUI Subsystem |
|---|---|---|---|---|---|
| **`baspp.exe` / `baspp`** | Flagship Desktop Edition | **640 MB** (`671088640L`) | `> ` | `Ok\n> ` | Full SDL2 graphics, BGI software rasterizer, TUI multiplexer, DAP server |
| **`bpp.exe` / `bpp`** | Lite REPL Edition | **384 MB** (`402653184L`) | `] ` | `Ready.\n] ` | Terminal console REPL. Weak symbol fallbacks in `vdev.c` for headless linking |
| **`bs.exe` / `bs`** | Batch Script Runner | **64 MB** (`67108864L`) | *None* | *None* | Headless non-interactive execution for PowerShell, Bash, CGI pipelines |
| **`iot.exe` / `iot`** | Microcontroller & IoT Edition | **2 MB** (`2097152L`) | `] ` | `Ready.\n] ` | Headless micro-REPL optimized for microcontrollers, hardware I/O, event loops |
| **`bppc.exe` / `bppc`** | Compiler & Transpiler | Dynamic | *Command-line* | *Diagnostics* | Standalone binary transpiler & bytecode emitter tool |
| **`detok.exe` / `detok`** | GW-BASIC Detokenizer | Dynamic | *Command-line* | *Text Output* | Decodes legacy GW-BASIC binary file format to text |
| **`trans.exe` / `trans`** | Source-to-Source Transpiler | Dynamic | *Command-line* | *Diagnostics* | Transpiles BASIC source to clean ISO C17 code |

---

## 3. Core Engine Subsystems & Architectural Features

### 3.1 Deterministic 9-Phase Bootloader Sequence
Bootstrapped via `boot_engine()` in `engine/src/bootstrap/common/common.c`:
1. **Phase 1: Platform Initialization (`plat_init`)** — Binds OS console, time, filesystem, and threading.
2. **Phase 2: Memory Partitioning (`mem_init`)** — Allocates standard memory pool (64 MB to 640 MB).
3. **Phase 3: Virtual Device Registration (`vdev_init`)** — Registers `CON:`, `KYBD:`, `SCRN:`, `PRN:`, `CAS1:`.
4. **Phase 4: Variable & String Heap Initialization (`var_table_init`, `str_pool_init`)** — Sets up symbol tables.
5. **Phase 5: Virtual Machine Context Creation (`vm_create`)** — Allocates `VMContext` state.
6. **Phase 6: Statement Registration (`stmt_register_all`)** — Registers all statement micro-libraries.
7. **Phase 7: Function Registration (`func_register_all`)** — Registers all 60 function micro-libraries.
8. **Phase 8: Capability Verification (`security_init`)** — Configures sandbox bitmask permissions.
9. **Phase 9: Environment Readiness (`vm_set_ready`)** — Initializes REPL prompt and looper readiness.

### 3.2 Non-Recursive Virtual Machine & Memory Management
- **Strict Non-Recursive Execution**: All AST evaluations, line executions, and bytecode loops operate on interpreter-managed heap stacks (`BValue` stack, line stack) with ZERO C recursion.
- **Reference-Counted String Heap**: Dynamic string handles managed via `str_retain()` and `str_release()`, preventing memory leaks on success and error paths.
- **Centralized Heap Tracker**: `MemoryContext` tracks all dynamic allocations, zero-initializing buffers by default (`calloc`/`memset`), and performing automated garbage collection on VM tear-down.

### 3.3 Virtual Device Bus (`VDev`) & Virtual Console (`VCon`)
- Pluggable device bus dispatch table supporting custom virtual devices (`CON:`, `KYBD:`, `SCRN:`, `PRN:`, `CAS1:`, `VNET:`, `Y:` device discovery).
- ANSI escape sequence parsing, screen buffer scrolling, cursor coordinate tracking (`g_cursor_x`, `g_cursor_y`), and color attribute mapping.

### 3.4 BGI Retro Display Engine & Pluggable BIOS Hal
- **42 Video Modes (`SET MODE`)**: Maps profile configurations across MDA, CGA, EGA, VGA, MCGA, Hercules, Atari 800 (`GRAPHICS 0..15`), Commodore 64, and Apple II retro display modes.
- **Tri-Mode BIOS HAL Dispatch**: Embedded freestanding `libbios` (PC, XT, AT, Jr) supporting C17 HLE, built-in 8086 micro-interpreter (`libcpu8086`), and direct VRAM observer callbacks (`0xB8000`, `0xA0000`).

### 3.5 Security Sandbox System (`SecurityContext`)
Configurable capability bitmask permissions:
- `SEC_PERM_FILE_READ` — Filesystem read operations.
- `SEC_PERM_FILE_WRITE` — Filesystem write/kill/mkdir operations.
- `SEC_PERM_EXEC` — Execution of external system commands (`SYSTEM`, `SHELL`).
- `SEC_PERM_NET` — Network socket access (`VNET:`).
- `SEC_PERM_RAW_MEM` — Direct hardware peek/poke memory access (`PEEK`, `POKE`).
*Violation triggers Error 70 (`ERR_PERMISSION_DENIED`).*

### 3.6 Dynamic Metaprogramming & Scope System
- `ALIAS` — Remaps keyword identifiers dynamically at runtime.
- `OVERRIDE` — Replaces statement execution handlers with custom BASIC or C logic.
- `SCOPE` — Establishes isolated lexical variable and statement visibility scopes.
- `KEYWORD` — Extends the lexer dictionary with new custom statements.
- `REMOVE` — Suppresses specified keywords from lexer parsing.

### 3.7 PRINT USING Formatting Engine
Complete format string specification parser in `engine/src/runtime/numfmt.c`:
- **Numeric Specifiers**: `#` (digit position), `.` (decimal point), `,` (thousands separator), `+` (leading/trailing sign), `-` (trailing negative sign), `$$` (floating dollar sign), `**$` (star fill with floating dollar), `**` (star fill), `^^^^` (exponential notation).
- **String Specifiers**: `!` (first character only), `\ \` (n+2 character field), `&` (variable length string field).
- **Literal Escaping**: `_` (escapes next literal character).

### 3.8 Cross-Language Interoperability Subsystem
- Interop handle table (`interop_handle.c`) for object reference tracking across host languages.
- Type marshaling (`interop_marshal.c`) supporting C, C++, C#, Python, and JS data types.
- JSON-RPC server (`interop_jsonrpc.c`) for remote execution pipelines.
- Win32 COM dispatch (`interop_com.c`) for OLE/COM object automation.
- Universal Compiler IR (`compiler_ir.c`) for native code generation passes.

---

## 4. Complete Inventory of Built-in Function Micro-Libraries (60 Functions)

All 60 built-in functions are implemented in `engine/src/eval/functions/` and register `MicroLibMetadata` signatures:

### 4.1 Bitwise Functions (7 Functions)
1. **`BITCOUNT(n)`** — Counts the number of set bits (population count) in an integer.
2. **`READBIT(n, b)`** — Reads the bit value (0 or 1) at bit position `b`.
3. **`RESETBIT(n, b)`** — Returns integer `n` with bit position `b` cleared (set to 0).
4. **`SETBIT(n, b)`** — Returns integer `n` with bit position `b` set to 1.
5. **`SHL(n, count)`** — Bitwise shift left of integer `n` by `count` bits.
6. **`SHR(n, count)`** — Bitwise shift right of integer `n` by `count` bits.
7. **`TOGGLEBIT(n, b)`** — Returns integer `n` with bit position `b` inverted.

### 4.2 Mathematical Functions (21 Functions)
8. **`ABS(x)`** — Absolute value of numeric expression `x`.
9. **`ACOS(x)`** — Arccosine of `x` in radians.
10. **`ASIN(x)`** — Arcsine of `x` in radians.
11. **`ATAN2(y, x)`** — Four-quadrant arctangent of `y/x` in radians.
12. **`ATN(x)`** — Arctangent of `x` in radians.
13. **`CEIL(x)`** — Smallest integer value greater than or equal to `x`.
14. **`CLAMP(x, min, max)`** — Constrains `x` within numeric bounds `[min, max]`.
15. **`COS(x)`** — Cosine of `x` in radians.
16. **`EXP(x)`** — Base-$e$ exponential $e^x$.
17. **`FIX(x)`** — Truncates `x` to an integer towards zero.
18. **`FLOOR(x)`** — Largest integer value less than or equal to `x`.
19. **`INT(x)`** — Floor integer conversion of numeric expression `x`.
20. **`LERP(a, b, t)`** — Linear interpolation between `a` and `b` by factor `t`.
21. **`LOG(x)`** — Natural logarithm (base-$e$) of `x`.
22. **`PI()`** — Mathematical constant $\pi \approx 3.141592653589793$.
23. **`RND([n])`** — Pseudo-random floating-point number in range `[0.0, 1.0)`.
24. **`ROUND(x [, decimals])`** — Rounds `x` to specified decimal precision.
25. **`SGN(x)`** — Signum function (-1 for negative, 0 for zero, 1 for positive).
26. **`SIN(x)`** — Sine of `x` in radians.
27. **`SQR(x)`** — Square root $\sqrt{x}$ (requires $x \ge 0$).
28. **`TAN(x)`** — Tangent of `x` in radians.

### 4.3 String Functions (19 Functions)
29. **`BIN$(n)`** — Returns binary string representation of integer `n`.
30. **`CHR$(code)`** — Returns single-character string matching ASCII code point.
31. **`HEX$(n)`** — Returns hexadecimal string representation of integer `n`.
32. **`INSTR([start,] str, search)`** — Position of substring `search` inside `str`.
33. **`LCASE$(str)`** — Converts string `str` to lower-case ASCII.
34. **`LEFT$(str, n)`** — Extracts leftmost `n` characters from `str`.
35. **`LEN(str)`** — Returns character length of string expression `str`.
36. **`LTRIM$(str)`** — Strips leading whitespace from `str`.
37. **`MID$(str, start [, length])`** — Extracts substring starting at `start`.
38. **`OCT$(n)`** — Returns octal string representation of integer `n`.
39. **`PACK$(fmt, val1, ...)`** — Binary data packing into a binary string.
40. **`RIGHT$(str, n)`** — Extracts rightmost `n` characters from `str`.
41. **`RTRIM$(str)`** — Strips trailing whitespace from `str`.
42. **`SPACE$(n)`** — Generates string of `n` spaces.
43. **`STR$(x)`** — Formats numeric expression `x` as a string.
44. **`STRING$(n, char)`** — Generates string of length `n` repeating character `char`.
45. **`TRIM$(str)`** — Strips leading and trailing whitespace from `str`.
46. **`UCASE$(str)`** — Converts string `str` to upper-case ASCII.
47. **`UNPACK$(fmt, str)`** — Unpacks binary data from string into variables.
48. **`VAL(str)`** — Parses numeric value from string `str`.

### 4.4 System & Environment Functions (11 Functions)
49. **`CLOCK()`** — Returns high-resolution numeric system clock timestamp.
50. **`CLOCK$()`** — Returns ISO 8601 formatted date/time string.
51. **`DATE$()`** — Returns current system date string (`MM-DD-YYYY`).
52. **`ENVIRON$(var)`** — Reads system environment variable value.
53. **`FRE(type)`** — Returns available free memory bytes in heap pool.
54. **`INKEY$()`** — Non-blocking single-character read from keyboard queue.
55. **`INP(port)`** — Reads 8-bit byte from I/O port address.
56. **`PEEK(addr)`** — Reads byte from direct VM memory address.
57. **`TICKS()`** — Returns system millisecond tick count since boot.
58. **`TIME$()`** — Returns current system time string (`HH:MM:SS`).
59. **`TIMER()`** — Returns seconds elapsed since midnight as a floating-point number.

### 4.5 Variable & Pointer Functions (2 Functions)
60. **`VARPTR(var)`** — Returns memory pointer offset of variable `var`.
61. **`VARPTR$(var)`** — Returns descriptor header string of variable `var`.

---

## 5. Complete Inventory of Implemented Statement Handlers & Commands

All statement handlers reside in `engine/src/statements/` and register syntax signatures in the VM statement registry:

### 5.1 Core Control Flow Statements
- **`END`** — Terminates program execution cleanly and releases context memory.
- **`GOSUB line`** — Pushes return address onto call stack and jumps to line.
- **`GOTO line`** — Unconditional jump to specified line number.
- **`IF expr THEN stmt [ELSE stmt]`** — Conditional branch evaluation. Supports single-line and block `IF...THEN...ELSE`.
- **`INPUT [prompt,] var1 [, var2...]`** — Interactive keyboard input with optional prompt.
- **`PRINT [expr_list]`** — Text output to virtual console `CON:`. Supports `;` (no newline), `,` (zone tab), and `USING` formatters.
- **`RANDOMIZE [seed]`** — Seeds the pseudo-random number generator.
- **`REM [comment]`** — Non-executing comment line.
- **`RETURN`** — Pops return address from call stack and resumes execution.
- **`SELECT CASE expr ... END SELECT`** — Multi-branch decision structure.
- **`STOP`** — Suspends program execution and enters debugging break mode.

### 5.2 Loops & Iteration
- **`FOR var = start TO end [STEP step]`** — Counter-based loop structure.
- **`NEXT [var]`** — Advances counter and loops back to corresponding `FOR`.
- **`WHILE expr ... WEND`** — Pre-condition loop while expression evaluates to true.
- **`DO [WHILE|UNTIL expr] ... LOOP [WHILE|UNTIL expr]`** — Flexible pre/post-condition loop structure.
- **`EXIT {FOR|WHILE|DO}`** — Immediate break out of active loop context.

### 5.3 Variables, Declarations & Memory
- **`LET var = expr`** — Variable assignment (optional `LET` keyword).
- **`DIM var(dim1 [, dim2...])`** — Array dimension allocation with option base awareness.
- **`ERASE array1 [, array2...]`** — Deallocates array memory buffers.
- **`OPTION BASE {0|1}`** — Sets default lower bound index for array dimensions.
- **`DEFINT letter_range`** — Sets default integer type for identifiers starting with specified letters.
- **`DEFSNG letter_range`** — Sets default single-precision float type.
- **`DEFDBL letter_range`** — Sets default double-precision float type.
- **`DEFSTR letter_range`** — Sets default string type.
- **`CLEAR [, [basic_bytes] [, stack_bytes]]`** — Re-initializes all program variables and clears stacks.
- **`COMMON var1 [, var2...]`** — Declares shared global variables passed across `CHAIN` calls.
- **`SHARED var1 [, var2...]`** — Exposes global variables inside subroutines/functions.
- **`DATA val1, val2...`** — Inline static data elements.
- **`READ var1 [, var2...]`** — Reads values sequentially from `DATA` statements.
- **`RESTORE [line]`** — Resets `DATA` reader pointer to beginning or specified line.
- **`SWAP var1, var2`** — Exchanges values between two variables.
- **`POKE addr, byte`** — Writes 8-bit byte to VM memory address.

### 5.4 Filesystem & File I/O Statements
- **`OPEN mode, [#]channel, filename [, rec_len]`** — Opens file for `INPUT`, `OUTPUT`, `APPEND`, `BINARY`, or `RANDOM` access.
- **`CLOSE [[#]channel1 [, [#]channel2...]]`** — Flushes and closes file channels.
- **`FIELD [#]channel, width AS var1 [, width AS var2...]`** — Binds random access buffer fields to string variables.
- **`FILES [pattern]`** — Displays directory file listing.
- **`GET [#]channel [, record_number]`** — Reads record from random access or binary file.
- **`PUT [#]channel [, record_number]`** — Writes record to random access or binary file.
- **`INPUT# channel, var1 [, var2...]`** — Reads formatted input from open file channel.
- **`PRINT# channel, [USING fmt;] expr_list`** — Writes formatted text output to file channel.
- **`WRITE# channel, expr_list`** — Writes comma-delimited raw data to file channel.
- **`SEEK [#]channel, position`** — Sets byte offset read/write position in open file.
- **`BLOAD filename [, offset]`** — Loads binary memory image into VM memory.
- **`BSAVE filename, offset, length`** — Saves VM memory block to binary file.
- **`KILL filename`** — Deletes file from filesystem.
- **`NAME old_name AS new_name`** — Renames file on filesystem.
- **`MKDIR path`** — Creates new directory.
- **`LOCK [#]channel [, record_range]`** — Locks file region for concurrent access.

### 5.5 BGI Graphics & Visual Display Statements
- **`SCREEN mode [, [colorswitch] [, [apage] [, vpage]]]`** — Sets video display mode (Modes 0..42).
- **`COLOR [fg] [, [bg] [, border]]`** — Sets active foreground, background, and border palette colors.
- **`CLS [mode]`** — Clears screen or text graphics window.
- **`HOME`** — Moves console cursor to top-left position (`0, 0`).
- **`LINE [(x1,y1)]-(x2,y2) [, [color] [, [BF] [, style]]]`** — Draws line, box (`B`), or filled box (`BF`).
- **`CIRCLE (x, y), radius [, [color] [, [start] [, [end] [, aspect]]]]`** — Draws circle or ellipse arc.
- **`PSET (x, y) [, color]`** — Sets pixel coordinate color.
- **`PRESET (x, y) [, color]`** — Resets pixel coordinate to background color.
- **`PAINT (x, y) [, [fill_color] [, border_color]]`** — Boundary flood fill algorithm.
- **`DRAW command_string`** — Executes turtle graphics vector drawing command language.
- **`PALETTE [attribute, color]`** — Customizes palette color hardware mappings.
- **`VIEW [[screen] (x1,y1)-(x2,y2) [, [fill] [, border]]]`** — Defines active graphics viewport boundary.
- **`WINDOW [[screen] (x1,y1)-(x2,y2)]`** — Sets world coordinate mapping transform.
- **`BGI command [, args]`** — Dispatches native BGI vector drawing and mode profiles.

### 5.6 Sound & Audio Statements
- **`BEEP`** — Generates standard 800 Hz alert tone sound.
- **`SOUND frequency, duration`** — Plays tone at frequency (Hz) for duration (clock ticks).
- **`PLAY music_string`** — Music Macro Language (MML) parser playing multi-voice melodies.
- **`VOICE voice_idx, wave_type`** — Sets synthesizer wave generator voice type.
- **`MUSIC command_string`** — Advanced multi-channel synthesizer control.

### 5.7 Events & Exception Handling Statements
- **`ON ERROR GOTO {line|0}`** — Enables global error trap handler line (or disables when 0).
- **`RESUME {NEXT|line|0}`** — Resumes execution after handling an error.
- **`TRY ... CATCH ... END TRY`** — Structured exception handling block.
- **`ON KEY(n) GOSUB line`** — Traps function key presses.
- **`ON TIMER(seconds) GOSUB line`** — Traps background timer intervals.
- **`ON COM(n) GOSUB line`** — Traps serial communication events.

### 5.8 Dialect, Metaprogramming & Introspection
- **`ALIAS "old_name" AS "new_name"`** — Remaps keyword identifiers dynamically at runtime.
- **`OVERRIDE "keyword" WITH handler`** — Replaces statement execution logic with custom handler.
- **`SCOPE {PUSH|POP|LOCAL} [scope_name]`** — Manages lexical variable/statement visibility scopes.
- **`KEYWORD "new_keyword"`** — Registers new statement keyword into lexer dictionary.
- **`REMOVE "keyword"`** — Suppresses keyword from lexer parsing.
- **`HELP [topic]`** — Interactive HELP system querying `MicroLibMetadata` registrations.
- **`INTROSPECT [subsystem]`** — Introspects VM symbol tables, memory state, and registered devices.
- **`SELFTEST`** — Executes built-in system self-diagnostics suite.

### 5.9 System, Concurrency & Security
- **`SECURITY level_bitmask`** — Configures sandbox security permission bitmask.
- **`SYSTEM`** — Halts VM looper and exits process to OS shell.
- **`TASK {RUN|KILL|LIST} [args]`** — Background task manager looper control.
- **`TEST suite_name`** — Runs automated test assertions.
- **`VERSION`** — Displays engine version and edition configuration details.

### 5.10 OOP & Structured Types
- **`TYPE type_name ... END TYPE`** — Defines User-Defined Type (UDT) struct layout.
- **`CLASS class_name ... END CLASS`** — Defines Object-Oriented Class structure.
- **`WITH object ... END WITH`** — Scoped property access block.

### 5.11 Transactions & Program Management
- **`TXN {BEGIN|COMMIT|ROLLBACK}`** — Transactional memory block control.
- **`RUN [line|filename] [, R]`** — Resets VM state and executes program from start or file (`, R` preserves `COMMON` variables and open file descriptors).
- **`LOAD filename`** — Loads BASIC program file into VM source buffer.
- **`SAVE filename`** — Saves current program source buffer to file.
- **`CHAIN [MERGE] filename [, [line] [, ALL] [, MERGE]]`** — Passes execution context to new BASIC program file, preserving `COMMON` variables by default (or all variables if `ALL` or `MERGE` option is specified).
- **`MERGE filename`** — Overlays program lines from file into active program memory without clearing variables or altering execution call stack.
- **`NEW`** — Clears active program buffer and resets variable symbol table.
- **`LIST [range]`** — Displays source line listing to console `CON:`.
- **`EDIT line`** — Opens interactive TUI editor on specified line.
- **`AUTO [start_line] [, increment]`** — Generates automatic line numbers for input.
- **`RENUM [new_start] [, [old_start] [, increment]]`** — Renumbers program lines and updates `GOTO`/`GOSUB`/`RESTORE` references across code.

---

## 6. Universal Dual-Format (Prefix & Infix) Engine

The engine provides full syntactic and semantic interchangeability between **infix** expressions and **prefix** functional calls across 19 mathematical, bitwise, and logical operations:

| Operation | Category | Infix Form | Prefix Form | Variadic? | 64-Bit Bounds |
|:---|:---|:---|:---|:---:|:---:|
| `AND` | Logic / Bitwise | `a AND b` | `AND(a, b [, ...])` | Yes | 64-bit (`int64_t`) |
| `OR` | Logic / Bitwise | `a OR b` | `OR(a, b [, ...])` | Yes | 64-bit (`int64_t`) |
| `XOR` | Logic / Bitwise | `a XOR b` | `XOR(a, b)` | No | 64-bit (`int64_t`) |
| `NOT` | Logic / Bitwise | `NOT a` | `NOT(a)` | No | 64-bit (`int64_t`) |
| `IMP` | Logic / Bitwise | `a IMP b` | `IMP(a, b)` | No | 64-bit (`int64_t`) |
| `EQV` | Logic / Bitwise | `a EQV b` | `EQV(a, b)` | No | 64-bit (`int64_t`) |
| `SHL` | Bitwise Shift | `a SHL b` | `SHL(a, b)` | No | 64-bit (`uint64_t`) |
| `SHR` | Bitwise Shift | `a SHR b` | `SHR(a, b)` | No | 64-bit (`uint64_t`) |
| `READBIT` | Bit Manipulation | `a READBIT b` | `READBIT(a, b)` | No | 64-bit (`uint64_t`) |
| `SETBIT` | Bit Manipulation | `a SETBIT b` | `SETBIT(a, b)` | No | 64-bit (`uint64_t`) |
| `RESETBIT` | Bit Manipulation | `a RESETBIT b` | `RESETBIT(a, b)` | No | 64-bit (`uint64_t`) |
| `TOGGLEBIT` | Bit Manipulation | `a TOGGLEBIT b` | `TOGGLEBIT(a, b)` | No | 64-bit (`uint64_t`) |
| `BITCOUNT` | Bit Manipulation | — | `BITCOUNT(a)` | No | 64-bit (`uint64_t`) |
| `MOD` | Math / Modulo | `a MOD b` | `MOD(a, b)` | No | 64-bit (`int64_t`) |
| `REMAINDER` | Math / Modulo | `a REMAINDER b` | `REMAINDER(a, b)` | No | Double Float (`IEEE 754`) |
| `MIN` | Math / Extrema | `a MIN b` | `MIN(a, b [, ...])` | Yes | Double Float |
| `MAX` | Math / Extrema | `a MAX b` | `MAX(a, b [, ...])` | Yes | Double Float |
| `HYPOT` | Math / Geometry | `a HYPOT b` | `HYPOT(a, b [, ...])` | Yes | Double Float |
| `ATAN2` | Math / Geometry | `y ATAN2 x` | `ATAN2(y, x)` | No | Double Float |

Every operator is registered as a standalone micro-library under `engine/src/eval/functions/` (`libengine`) and is queryable via interactive `HELP` and `CATALOG`.



---

## 13. Systems Programming, Freestanding C17, Bare-Metal UEFI & Object Pascal Architecture

### 13.1 Systems Programming Compilation Pipeline (`bppc` & `trans`)
BASIC++ provides a first-class native compilation pipeline capable of producing genuine freestanding binaries with zero interpreter, zero garbage collector, and zero hosted C runtime dependencies:

1. **Freestanding C17 Target (`--freestanding`)**:
   - Emits pure ISO C17 code requiring zero libc runtime functions (`-ffreestanding -nostdlib`).
   - Sized ordinal primitive types: `U8`, `I8`, `U16`, `I16`, `U32`, `I32`, `U64`, `I64`, `USIZE`, `ISIZE`.
   - Bare-metal entry point `void _start(void)`.
   - Volatile hardware memory and port access: `BPP_MEM8(addr)`, `BPP_MEM16(addr)`, `BPP_MEM32(addr)`, `BPP_MEM64(addr)`, `BPP_PORT8(port)`.
   - Syntactic hardware arrays: `MEM[addr]`, `MEMW[addr]`, `MEML[addr]`, `PORT[port]` using reserved square brackets `[ ]`.
   - Direct inline assembly escape: `ASM "..."`.

2. **64-bit Bare-Metal UEFI Target (`--uefi`)**:
   - Compiles directly into 64-bit UEFI firmware application payloads (`\EFI\BOOT\BOOTX64.EFI`).
   - Entry point: `EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)`.
   - Native UEFI text console protocols: `EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL` providing `OutputString` (UCS-2 / UTF-16LE conversion) and `ClearScreen`.
   - Enables machine cold boot directly into a native BASIC++ systems environment with zero operating system beneath it.

3. **Object Pascal Transpilation Target (`--pascal`)**:
   - Generates clean Object Pascal targeting the Free Pascal Compiler (FPC).
   - Generates `program GeneratedBppProgram;` with `{$mode objfpc}{$H+}`.
   - Maps sized types to Pascal primitives (`Byte`, `ShortInt`, `Word`, `SmallInt`, `LongWord`, `LongInt`, `QWord`, `Int64`, `PtrUInt`, `PtrInt`).
   - Direct typed pointer dereferencing: `PByte(PtrUInt(addr))^ := val;`.

---

## 13. Systems Programming, Freestanding C17, Bare-Metal UEFI & Object Pascal Architecture

### 13.1 Systems Programming Compilation Pipeline (`bppc` & `trans`)
BASIC++ provides a first-class native compilation pipeline capable of producing genuine freestanding binaries with zero interpreter, zero garbage collector, and zero hosted C runtime dependencies:

1. **Freestanding C17 Target (`--freestanding`)**:
   - Emits pure ISO C17 code requiring zero libc runtime functions (`-ffreestanding -nostdlib`).
   - Sized ordinal primitive types: `U8`, `I8`, `U16`, `I16`, `U32`, `I32`, `U64`, `I64`, `USIZE`, `ISIZE`.
   - Bare-metal entry point `void _start(void)`.
   - Volatile hardware memory and port access: `BPP_MEM8(addr)`, `BPP_MEM16(addr)`, `BPP_MEM32(addr)`, `BPP_MEM64(addr)`, `BPP_PORT8(port)`.
   - Syntactic hardware arrays: `MEM[addr]`, `MEMW[addr]`, `MEML[addr]`, `PORT[port]` using reserved square brackets `[ ]`.
   - Direct inline assembly escape: `ASM "..."`.

2. **64-bit Bare-Metal UEFI Target (`--uefi`)**:
   - Compiles directly into 64-bit UEFI firmware application payloads (`\EFI\BOOT\BOOTX64.EFI`).
   - Entry point: `EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)`.
   - Native UEFI text console protocols: `EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL` providing `OutputString` (UCS-2 / UTF-16LE conversion) and `ClearScreen`.
   - Enables machine cold boot directly into a native BASIC++ systems environment with zero operating system beneath it.

3. **Object Pascal Transpilation Target (`--pascal`)**:
   - Generates clean Object Pascal targeting the Free Pascal Compiler (FPC).
   - Generates `program GeneratedBppProgram;` with `{$mode objfpc}{$H+}`.
   - Maps sized types to Pascal primitives (`Byte`, `ShortInt`, `Word`, `SmallInt`, `LongWord`, `LongInt`, `QWord`, `Int64`, `PtrUInt`, `PtrInt`).
   - Direct typed pointer dereferencing: `PByte(PtrUInt(addr))^ := val;`.

---

## 14. Historical Dialect Feature Import: Vintage Ecosystems & Apple III Business Extensions (Waves 1–12)

### 14.1 Phase 2A (Waves 1–4): Freestanding Linear Algebra, Slicing & Pick MultiValue
1. **Wave 1: Freestanding Linear Algebra & Bare DET**:
   - `DET` / `MATH.DET`: Bare identifier returning determinant of the last inverted matrix (`MAT N = INV(M)`) via Gaussian elimination.
   - `DOT(u, v)`: Vector dot product with support for `VAL_NUMBER` and `VAL_INTEGER` array elements across 0-based and 1-based indexing.

2. **Wave 2: Slicing, Codecs, and Low-Memory Storage**:
   - Bracket Slicing (`S$[start TO end]`, `S$[start, end]`, `S$[start]`): Atari and HP-style zero-copy string slicing.
   - `CHANGE` (DEC PDP-10 / Tymshare Super BASIC): `CHANGE S$ TO A%` and `CHANGE A% TO S$`, dynamically respecting `OPTION BASE 0` (`A%(0)` is length) and `OPTION BASE 1` (`A%(1)` is length) with stack/arena memory.
   - `PACK` & `UNPACK`: Dedicated statement synonyms for string-array ASCII conversion.
   - `EXAM(addr)` & `FILL addr, val`: North Star BASIC byte read function and write statement with 64KB virtual low RAM emulation.

3. **Wave 3: Fractional & Financial Rounding Intrinsics**:
   - `FRAC(x)` / `FP(x)`: Returns fractional part of a number (`x - trunc(x)`).
   - `BANKER_ROUND(x [, n])`: IEEE 754 round half to even (banker's rounding).

4. **Wave 4: Pick MultiValue Dynamic Arrays & Unified Functions**:
   - Option A (Primary): Context-disambiguated `LOCATE target IN arr SETTING pos THEN ... ELSE ...` (disambiguated from QuickBASIC screen `LOCATE row, col`).
   - Option B (Synonym): `FIND target IN arr SETTING pos THEN ... ELSE ...` (disambiguated from DEC file `FIND`).
   - Option C (Compound): `ARRAY FIND target IN arr SETTING pos THEN ... ELSE ...`.
   - Sorted ordering: `LOCATE target IN arr BY "AL"|"AR"|"DL"|"DR" SETTING pos`.
   - Unified Array & String Functions: `INDEX(arr, target)` / `INDEX$(arr, target)`, `INSERT(arr, pos, val)` / `INSERT$(arr, pos, val$)`, `REPLACE(arr, pos, val)` / `REPLACE$(arr, pos, val$)`, `DELETE(arr, pos)` / `DELETE$(arr, pos)` / `REMOVE$(arr, pos)`, `COUNT(arr [, delim$])` / `COUNT$(arr [, delim$])`, `FIELD(dyn, delim$, idx)`, `EXTRACT(dyn, am [, vm [, svm]])`. Legacy `D`-prefixed functions are shelved in `planned/`.

### 14.2 Phase 2B (Waves 5–8): Bitwise Registers, Sized Strings, Reductions & Renumbering
1. **Wave 5: Bitwise Registers & Bitfield Ops**:
   - `BIT(val, bit)` (alias for `READBIT`), `CLRBIT(val, bit)` (alias for `RESETBIT`), `SETBIT`, `TOGGLEBIT`.
   - `ROL(val, count [, width])`: Rotate bits left with optional width (default 32).
   - `ROR(val, count [, width])`: Rotate bits right with optional width (default 32).
   - `BITFIELD(val, start, len)`: Extract bitfield slice from integer value.
   - Bracket modifiers supported across bitwise functions: `ROL[...]`, `ROR[...]`, `BITFIELD[...]`.

2. **Wave 6: String Length Sizing & Dynamic Descriptors**:
   - String length constraints on declaration: `DIM S$ = len`, `DIM S$[len]`, `DIM S$ * len`, `DIM S AS STRING * len`, `DIM S AS STRING = len`, `DIM S AS STRING[len]`.
   - Automatic string clamping/truncation upon variable assignment when string length exceeds declared `max_len`.
   - `MAXLEN(S$)` / `MAXLEN[S$]`: Retrieves declared maximum length constraint of sized string variable (0 if unbounded).

3. **Wave 7: Mathematical Reductions & Statistics (Tri-Hybrid Delimiters `()`, `[]`, `{}`)**:
   - `SUM(...)`: Variadic summation of numeric arguments, array reduction `SUM(arr)` / `SUM[arr]`, and set collection `SUM{...}`.
   - `AVG(...)` / `MEAN(...)`: Arithmetic mean of numeric arguments, array reduction `AVG(arr)` / `AVG[arr]`, and set collection `AVG{...}`.
   - `MIN(...)` / `MAX(...)`: Minimum and maximum of numeric arguments, array reduction `MIN(arr)` / `MIN[arr]`, and set collection `MIN{...}`.

4. **Wave 8: Auto-Renumbering Preprocessor**:
   - File-to-file renumbering preprocessor: `RENUM "input.bas", start, step TO "output.bas"`.
   - In-place file renumbering: `RENUM "input.bas", start, step`.
   - Automated line numbering on load: `LOAD "input.bas"`, `RENUM` automatically assigns deterministic line numbers (10, 20, 30...) to unnumbered source files.

### 14.3 Phase 2C (Waves 9–12): Extended Vintage Ecosystems & Apple III Business Extensions
1. **Wave 9: Sharp Pocket Trigonometric Angle Modes & Polar/Rectangular Coordinates**:
   - `DEGREE`, `RADIAN`, `GRAD`: Statements to switch global VM trigonometric angle mode.
   - `DEGREE(rad)`, `RADIAN(deg)`, `GRAD(deg)`: Explicit angle conversion functions.
   - Angle-Aware Trig: `SIN`, `COS`, `TAN`, `ASIN`, `ACOS`, `ATN` dynamically convert angles based on active VM angle mode across both interpreter and AST evaluators.
   - `POL(x, y [, coord])` / `POL[x, y]`: Polar coordinate conversion returning radius (r = hypot(x, y)) or angle (theta = atan2(y, x) in active angle mode).
   - `REC(r, theta [, coord])` / `REC[r, theta]`: Rectangular coordinate conversion returning x (r * cos(theta)) or y (r * sin(theta)) in active angle mode.

2. **Wave 10: Tektronix 4050 Terminal Control, GPIB Bus & Orthogonal Brackets**:
   - `PAGE`: Clear screen statement via virtual console (`vcon_clear_screen`, `vcon_locate`, `\f`).
   - `WBYTE`, `RBYTE`: GPIB / IEEE-488 instrument I/O statements and functions: `WBYTE data...`, `WBYTE @dev, sec: data...`, `WBYTE[dev, sec] data...`, `WBYTE(dev, data)`, `RBYTE var...`, `RBYTE @dev, sec: var...`, `RBYTE[dev, sec] var...`, `RBYTE(dev)`, backed by a virtual GPIB bus controller and FIFO buffer ring.
   - Orthogonal Bracket Coordinates: `WINDOW [xmin..xmax, ymin..ymax]` and `VIEWPORT [xmin..xmax, ymin..ymax]` adhering strictly to square-bracket delimiter semantics.

3. **Wave 11: Wang 2200 Substring Search & Character Translation**:
   - `POS(s$, target$ [, start])`: Substring position search with optional 1-based start offset, coexisting with legacy console column check `POS(0)`.
   - `TRANSLATE var$ USING table$`, `TRANSLATE var$, from$, to$`, `TRANSLATE[from$, to$] var$`: In-place character translation statement using lookup tables or character pairs.
   - `TRANSLATE$(src$, from$, to$)`, `TRANSLATE$(src$, table$)`, `TRANSLATE$[from$, to$](src$)`: Character translation function returning mapped string.

4. **Wave 12: Business BASIC & Apple III Terminal Control & Formatting**:
   - `@(col, row)` / `@[col, row]`: 2D screen cursor positioning within `PRINT` statements.
   - `@(-1)`: Clear screen (`CLS`) terminal control mnemonic.
   - `@(-2)`: Cursor home (`HOME`) terminal control mnemonic.
   - `@(-3)`: Erase to end of line (`EOL`) terminal control mnemonic.
   - `@(-4)`: Erase to end of screen (`EOS`) terminal control mnemonic.

---

## 14. Historical Dialect Feature Import: Vintage Ecosystems & Apple III Business Extensions (Waves 1–12)

### 14.1 Phase 2A (Waves 1–4): Freestanding Linear Algebra, Slicing & Pick MultiValue
1. **Wave 1: Freestanding Linear Algebra & Bare DET**:
   - `DET` / `MATH.DET`: Bare identifier returning determinant of the last inverted matrix (`MAT N = INV(M)`) via Gaussian elimination.
   - `DOT(u, v)`: Vector dot product with support for `VAL_NUMBER` and `VAL_INTEGER` array elements across 0-based and 1-based indexing.

2. **Wave 2: Slicing, Codecs, and Low-Memory Storage**:
   - Bracket Slicing (`S$[start TO end]`, `S$[start, end]`, `S$[start]`): Atari and HP-style zero-copy string slicing.
   - `CHANGE` (DEC PDP-10 / Tymshare Super BASIC): `CHANGE S$ TO A%` and `CHANGE A% TO S$`, dynamically respecting `OPTION BASE 0` (`A%(0)` is length) and `OPTION BASE 1` (`A%(1)` is length) with stack/arena memory.
   - `PACK` & `UNPACK`: Dedicated statement synonyms for string-array ASCII conversion.
   - `EXAM(addr)` & `FILL addr, val`: North Star BASIC byte read function and write statement with 64KB virtual low RAM emulation.

3. **Wave 3: Fractional & Financial Rounding Intrinsics**:
   - `FRAC(x)` / `FP(x)`: Returns fractional part of a number (`x - trunc(x)`).
   - `BANKER_ROUND(x [, n])`: IEEE 754 round half to even (banker's rounding).

4. **Wave 4: Pick MultiValue Dynamic Arrays & Unified Functions**:
   - Option A (Primary): Context-disambiguated `LOCATE target IN arr SETTING pos THEN ... ELSE ...` (disambiguated from QuickBASIC screen `LOCATE row, col`).
   - Option B (Synonym): `FIND target IN arr SETTING pos THEN ... ELSE ...` (disambiguated from DEC file `FIND`).
   - Option C (Compound): `ARRAY FIND target IN arr SETTING pos THEN ... ELSE ...`.
   - Sorted ordering: `LOCATE target IN arr BY "AL"|"AR"|"DL"|"DR" SETTING pos`.
   - Unified Array & String Functions: `INDEX(arr, target)` / `INDEX$(arr, target)`, `INSERT(arr, pos, val)` / `INSERT$(arr, pos, val$)`, `REPLACE(arr, pos, val)` / `REPLACE$(arr, pos, val$)`, `DELETE(arr, pos)` / `DELETE$(arr, pos)` / `REMOVE$(arr, pos)`, `COUNT(arr [, delim$])` / `COUNT$(arr [, delim$])`, `FIELD(dyn, delim$, idx)`, `EXTRACT(dyn, am [, vm [, svm]])`. Legacy `D`-prefixed functions are shelved in `planned/`.

### 14.2 Phase 2B (Waves 5–8): Bitwise Registers, Sized Strings, Reductions & Renumbering
1. **Wave 5: Bitwise Registers & Bitfield Ops**:
   - `BIT(val, bit)` (alias for `READBIT`), `CLRBIT(val, bit)` (alias for `RESETBIT`), `SETBIT`, `TOGGLEBIT`.
   - `ROL(val, count [, width])`: Rotate bits left with optional width (default 32).
   - `ROR(val, count [, width])`: Rotate bits right with optional width (default 32).
   - `BITFIELD(val, start, len)`: Extract bitfield slice from integer value.
   - Bracket modifiers supported across bitwise functions: `ROL[...]`, `ROR[...]`, `BITFIELD[...]`.

2. **Wave 6: String Length Sizing & Dynamic Descriptors**:
   - String length constraints on declaration: `DIM S$ = len`, `DIM S$[len]`, `DIM S$ * len`, `DIM S AS STRING * len`, `DIM S AS STRING = len`, `DIM S AS STRING[len]`.
   - Automatic string clamping/truncation upon variable assignment when string length exceeds declared `max_len`.
   - `MAXLEN(S$)` / `MAXLEN[S$]`: Retrieves declared maximum length constraint of sized string variable (0 if unbounded).

3. **Wave 7: Mathematical Reductions & Statistics (Tri-Hybrid Delimiters `()`, `[]`, `{}`)**:
   - `SUM(...)`: Variadic summation of numeric arguments, array reduction `SUM(arr)` / `SUM[arr]`, and set collection `SUM{...}`.
   - `AVG(...)` / `MEAN(...)`: Arithmetic mean of numeric arguments, array reduction `AVG(arr)` / `AVG[arr]`, and set collection `AVG{...}`.
   - `MIN(...)` / `MAX(...)`: Minimum and maximum of numeric arguments, array reduction `MIN(arr)` / `MIN[arr]`, and set collection `MIN{...}`.

4. **Wave 8: Auto-Renumbering Preprocessor**:
   - File-to-file renumbering preprocessor: `RENUM "input.bas", start, step TO "output.bas"`.
   - In-place file renumbering: `RENUM "input.bas", start, step`.
   - Automated line numbering on load: `LOAD "input.bas"`, `RENUM` automatically assigns deterministic line numbers (10, 20, 30...) to unnumbered source files.

### 14.3 Phase 2C (Waves 9–12): Extended Vintage Ecosystems & Apple III Business Extensions
1. **Wave 9: Sharp Pocket Trigonometric Angle Modes & Polar/Rectangular Coordinates**:
   - `DEGREE`, `RADIAN`, `GRAD`: Statements to switch global VM trigonometric angle mode.
   - `DEGREE(rad)`, `RADIAN(deg)`, `GRAD(deg)`: Explicit angle conversion functions.
   - Angle-Aware Trig: `SIN`, `COS`, `TAN`, `ASIN`, `ACOS`, `ATN` dynamically convert angles based on active VM angle mode across both interpreter and AST evaluators.
   - `POL(x, y [, coord])` / `POL[x, y]`: Polar coordinate conversion returning radius (r = hypot(x, y)) or angle (theta = atan2(y, x) in active angle mode).
   - `REC(r, theta [, coord])` / `REC[r, theta]`: Rectangular coordinate conversion returning x (r * cos(theta)) or y (r * sin(theta)) in active angle mode.

2. **Wave 10: Tektronix 4050 Terminal Control, GPIB Bus & Orthogonal Brackets**:
   - `PAGE`: Clear screen statement via virtual console (`vcon_clear_screen`, `vcon_locate`, `\f`).
   - `WBYTE`, `RBYTE`: GPIB / IEEE-488 instrument I/O statements and functions: `WBYTE data...`, `WBYTE @dev, sec: data...`, `WBYTE[dev, sec] data...`, `WBYTE(dev, data)`, `RBYTE var...`, `RBYTE @dev, sec: var...`, `RBYTE[dev, sec] var...`, `RBYTE(dev)`, backed by a virtual GPIB bus controller and FIFO buffer ring.
   - Orthogonal Bracket Coordinates: `WINDOW [xmin..xmax, ymin..ymax]` and `VIEWPORT [xmin..xmax, ymin..ymax]` adhering strictly to square-bracket delimiter semantics.

3. **Wave 11: Wang 2200 Substring Search & Character Translation**:
   - `POS(s$, target$ [, start])`: Substring position search with optional 1-based start offset, coexisting with legacy console column check `POS(0)`.
   - `TRANSLATE var$ USING table$`, `TRANSLATE var$, from$, to$`, `TRANSLATE[from$, to$] var$`: In-place character translation statement using lookup tables or character pairs.
   - `TRANSLATE$(src$, from$, to$)`, `TRANSLATE$(src$, table$)`, `TRANSLATE$[from$, to$](src$)`: Character translation function returning mapped string.

4. **Wave 12: Business BASIC & Apple III Terminal Control & Formatting**:
   - `@(col, row)` / `@[col, row]`: 2D screen cursor positioning within `PRINT` statements.
   - `@(-1)`: Clear screen (`CLS`) terminal control mnemonic.
   - `@(-2)`: Cursor home (`HOME`) terminal control mnemonic.
   - `@(-3)`: Erase to end of line (`EOL`) terminal control mnemonic.
   - `@(-4)`: Erase to end of screen (`EOS`) terminal control mnemonic.
### 14.4 Phase 2D (Waves 13–16): HAL-Mediated & Systems Integration
1. **Wave 13: Pick & Business BASIC Record Locking**:
   - `READU`: Dynamic record reading with exclusive lock acquisition across multiple syntaxes: `READU [#]ch, id, var$`, `READU[ch, id] var$`, `READU var$ FROM [#]ch, id`, and function forms `status = READU(ch, id, var$)`, `rec$ = READU$(ch, id)`.
   - `WRITEU`: Record write statement retaining active record lock: `WRITEU [#]ch, id, data$`, `WRITEU[ch, id] data$`, `WRITEU data$ ON/TO [#]ch, id`, and function form `status = WRITEU(ch, id, data$)`.
   - `RELEASE`: Explicit lock release statement and function: bare `RELEASE`, `RELEASE [#]ch`, `RELEASE [#]ch, id`, `RELEASE[ch, id]`, `status = RELEASE(ch, id)`, `RELEASE(ch)`, `RELEASE()`.
   - `LOCKED`: Intrinsic predicate function returning `-1` (true) if record is locked or `0` (false) if unlocked: `is_locked = LOCKED(ch, id)` and `LOCKED[ch, id]`. Backed by an $O(1)$ pre-allocated static lock table (128 slots) with zero bare `malloc`.

2. **Wave 14: Structured ISAM & Keyed Storage Abstract HAL Core**:
   - High-level ISAM & Keyed Storage HAL abstraction with multi-syntax statement operations: `OPEN "file.db" AS #ch KEYED`, `SEEKEQ [#]ch, key$`, `SEEKEQ[ch] key$`, `SEEKGE [#]ch, key$`, `SEEKGE[ch] key$`, `RETRIEVE [#]ch, key$, var$`, `RETRIEVE[ch] key$, var$`, `UPDATE [#]ch, key$, data$`, `UPDATE[ch] key$, data$`, `INSERT [#]ch, key$, data$`, `INSERT[ch] key$, data$`.
   - ISAM Intrinsic Query Functions: `KEY$(ch)` returns current active key or primary index; `KEYCOUNT(ch)` and `KEYCOUNT[ch]` return total indexed key count; `ISAM(ch)` and `KEYED(ch)` return boolean status (`-1` / `0`) verifying active indexed file mode.

3. **Wave 15: Non-BASIC Systems Types & Concurrency Extensions**:
   - Explicit Non-BASIC Systems Types: `CBYTE(val)`, `BYTE(val)`, `CWORD(val)`, `WORD(val)`, `CDWORD(val)`, `DWORD(val)` providing integer truncations to 8-bit, 16-bit, and 32-bit unsigned/signed representations.
   - Bracket Memory Width Accessors: `BYTE[addr]`, `WORD[addr]`, `DWORD[addr]` providing physical memory peek accessors with width-directed bounds checking and memory barriers.
   - Low-Level Pointer & Address Descriptors: `PTR(var)` returns virtual address / memory handle; `DEREF(addr)` dereferences byte value at virtual address.
   - Bitfield & Masking Operations: `BITFIELD(val, start, len)` and curried `BITFIELD[start, len](val)`; `SET_BITFIELD(val, start, len, new_bits)`; `MASK(val, mask_pattern)` and curried `MASK[mask_pattern](val)`.
   - Concurrency & Mutex Primitives: `MUTEX "INIT", id`, `MUTEX "LOCK", id`, `MUTEX "UNLOCK", id`; functional acquisition: `m = MUTEX(name$)`, `MUTEX_LOCK(id)`, `MUTEX_UNLOCK(id)` backed by pre-allocated static mutex pool with auto-activation.

4. **Wave 16: Master Dialect Regression & Closure**:
   - 100% dialect test suite coverage across GW-BASIC, QBASIC, Super BASIC, Pick MultiValue, and ECMA-116.
   - Tri-hybrid delimiter semantic invariants rigorously verified: `( )` for infix function calls and grouping, `[ ]` for bracket slicing, Polish notation, and memory width modifiers, `{ }` for reverse Polish notation, JSON dictionaries, and variadic sets.
   - Unified array and string functions verified without duplication: `INDEX`, `INDEX$`, `INSERT`, `INSERT$`, `REPLACE`, `REPLACE$`, `REMOVE`, `REMOVE$`, `DELETE`, `DELETE$`, `COUNT`, `COUNT$`. Redundant `D`-prefixed and `P`-prefixed functions purged and shelved.

---

## 15. Stage 2: Python 3 Capability Import and Set-Based Object Model

### 15.1 Wave 1: Core Set Model & Algebraic Engine
1. **Foundational Principle**:
   - "Everything is a Set, and what other languages call an Object is a Group within a Set."
   - First-class Set collection (`VAL_SET`, `BppSet`) and Group collection (`VAL_GROUP`, `BppGroup`) types seamlessly integrated into the unified `BValue` runtime type system.

2. **Braced Set and Group Literals**:
   - Braced Set Literals: `{ 1, 2, 3 }`, `{ "apple", "banana" }`, and empty set `{}`. Automatically deduplicates elements upon insertion.
   - Braced Group Literals: `{ name: "Alice", age: 30, city: "Denver" }` storing key-value pairs where keys are string identifiers and values are first-class `BValue` payloads.

3. **Algebraic Set Operators**:
   - **Union (`|`)**: `u = a | b` produces a new Set containing all unique elements from both sets.
   - **Intersection (`&`)**: `inter = a & b` produces a new Set containing elements present in both sets.
   - **Set Difference (`\`)**: `diff = a \ b` produces a new Set containing elements present in `a` but not in `b`. Contextually and syntactically disambiguated from numeric integer division (`10 \ 3 = 3`) and vintage DEC PDP-11 statement separators.
   - **Symmetric Difference (`^`)**: `sdiff = a ^ b` produces a new Set containing elements present in either set but not in both. Disambiguated from power exponentiation (`2 ^ 8 = 256`).
   - **Subset & Proper Subset (`<=`, `<`)**: `sub1 <= a` and `sub1 < a` evaluate subset relations returning relational truth values (`-1` for true, `0` for false).
   - **Set Equality & Relational Testing (`=`, `<>`)**: Evaluates deep structural element equality across sets (`-1` for true, `0` for false).
   - **Set Membership (`IN`)**: `item IN S` tests element containment returning `-1` (true) or `0` (false), contextually disambiguated from Pick MultiValue `LOCATE target IN arr SETTING pos`.

4. **Addressing, Indexing, and In-Place Mutation**:
   - Cardinality & Element Count: `COUNT(S)` returns set cardinality; `COUNT(S, item)` returns occurrences (`0` or `1`).
   - Group Key Addressing & Mutation: `g{"key"}` retrieves property value; `g{"key"} = val` mutates or adds property in-place.
   - Set 1-Based Element Addressing & Mutation: `S{idx}` retrieves element by index; `S{idx} = val` updates element in-place.
   - Array Brace Addressing Parity: `arr{1} = 42` provides orthogonal brace addressing into dimensioned arrays.

5. **Certified Memory Allocation & ISO C17 Safety**:
   - Zero bare `malloc`: Implemented strictly using pool allocation (`RuntimePool`) and monotonic arenas (`RuntimeArena`) conforming to ISO C17 (§4 ¶6) freestanding rules.

### 15.2 Wave 2: Pick MultiValue Dynamic Array Group Mapping & Map Container Unification
1. **Container Subsumption & Unification**:
   - `BppGroup` (`VAL_GROUP`) seamlessly subsumes associative key-value maps (`VAL_MAP`), allowing unified key-to-value addressing and mutation.
   - Bidirectional conversion helpers (`group_from_map`, `map_from_group`) bridge legacy associative arrays with first-class set algebra.

2. **Group and Map Algebraic Operations**:
   - **Union (`|`)**: `G1 | G2` merges key-value pairs; right-operand values take precedence for shared keys with full $O(N)$ hash insertion.
   - **Intersection (`&`)**: `G1 & G2` retains entries whose keys are present in both groups, adopting the right operand's value for consistent precedence.
   - **Difference (`\`)**: `G1 \ G2` extracts entries whose keys exist in `G1` but are absent in `G2`.
   - **Symmetric Difference (`^`)**: `G1 ^ G2` yields entries present in either group but not in both.
   - **Subset & Proper Subset (`<=`, `<`)**: `G1 <= G2` returns `-1` (true) if all entries (keys and structural values) in `G1` are identically contained in `G2`.
   - **Structural Equality (`=`, `<>`)**: Evaluates deep structural match across all group keys and values.

3. **Multi-Tier Hierarchical Path Addressing & Mutation**:
   - Multi-tier brace access: `R{"dept", "mgr", "name"}` descends nested Groups, Maps, and Sets with arbitrary depth.
   - Multi-tier in-place mutation: `R{"emp", "salary"} = 100000` traverses or lazily constructs intermediate groups and mutates the target value in-place.

4. **Pick MultiValue Three-Tier Dynamic Array Integration**:
   - Native 3-tier dynamic array representation: Attribute Marks (`CHR$(254)` / `@AM`), Value Marks (`CHR$(253)` / `@VM`), and Subvalue Marks (`CHR$(252)` / `@SVM`).
   - String Path Extraction: `dyn${attr [, val [, subval]]}` performs $O(1)$ substring extraction using orthogonal brace syntax.
   - Dynamic String Mutation: `dyn${attr [, val [, subval]]} = "new_val"` reconstructs and replaces delimited segments in-place.
   - `PARSE_DYNARRAY(dyn$)` / `GROUP_MAP(dyn$)`: Parses Pick dynamic array string into a 3-tier hierarchical Set of Groups.
   - `DYNARRAY$(G)` / `DYNARRAY(G)`: Serializes a Set or Group back into a Pick-delimited string with 100% binary fidelity.
   - Polymorphic Pick Intrinsics: `EXTRACT`, `REPLACE`, `INSERT`, `DELETE` operate interchangeably over dynamic strings, Sets, and Groups.

5. **Relational Wildcard and Predicate Projections**:
   - Wildcard Attribute Projection: `S{*, "id"}` projects attribute across all group items in the set.
   - Predicate Filter Projection: `S{*, "role" = "user", "id"}` filters set elements matching predicate criteria and projects the target field into a new Set.
