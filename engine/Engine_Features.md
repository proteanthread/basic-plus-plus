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

BASIC++ builds into five distinct executable binary targets optimized for different execution environments:

| Target Executable | Edition Name | Default Memory Pool | Interactive Prompt | Output Status | Graphic/TUI Subsystem |
|---|---|---|---|---|---|
| **`baspp.exe` / `baspp`** | Flagship Desktop Edition | **640 MB** (`671088640L`) | `> ` | `Ok\n> ` | Full SDL2 graphics, BGI software rasterizer, TUI multiplexer, DAP server |
| **`bpp.exe` / `bpp`** | Lite REPL Edition | **384 MB** (`402653184L`) | `] ` | `Ready.\n] ` | Terminal console REPL. Weak symbol fallbacks in `vdev.c` for headless linking |
| **`bs.exe` / `bs`** | Batch Script Runner | **64 MB** (`67108864L`) | *None* | *None* | Headless non-interactive execution for PowerShell, Bash, CGI pipelines |
| **`bppc.exe` / `bppc`** | Compiler & Transpiler | Dynamic | *Command-line* | *Diagnostics* | Standalone binary transpiler & bytecode emitter tool |
| **`detok.exe` / `detok`** | GW-BASIC Detokenizer | Dynamic | *Command-line* | *Text Output* | Decodes legacy GW-BASIC binary file format to text |

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


