# TRON

### 1. Syntax & Parameters
`TRON`
`TROFF`

Parameters:
- None.

### 2. Description & Usage
The `TRON` (Trace On) command enables the built-in execution tracing facility of the BASIC++ interpreter, which is an invaluable debugging tool for tracking the flow of control within a program. Once `TRON` is executed, the interpreter automatically outputs the line number of each subsequent line of code as it is executed. The line numbers are typically enclosed in square brackets, such as `[10]`, and are printed directly to the active output stream or console window. 

Tracing remains active until it is explicitly disabled by the complementary `TROFF` (Trace Off) command, or until the interpreter undergoes a complete reset (such as when `NEW` is executed).

**Usage:**
- Insert `TRON` right before a suspected problematic block of code to monitor execution flow.
- Combine with `TROFF` immediately after the block to limit the trace output and prevent console flooding.
- Useful for diagnosing infinite loops, incorrect branching (e.g., `GOTO`, `GOSUB`, `IF...THEN`), and unexpected subroutine returns.

**Edge Cases & Boundary Conditions:**
- **Execution in Immediate Mode:** If `TRON` is entered in immediate (direct) mode, tracing will be enabled for the next executed program (e.g., upon typing `RUN`).
- **No Line Numbers:** If the program is written using modern block-structured BASIC without line numbers, `TRON` will trace execution by outputting internal program counter offsets or logical line indices, depending on the interpreter's configuration.
- **Nested Includes:** When tracing enters an included file or external module, the line numbers might reset or include a file identifier, depending on the VFS (Virtual File System) mapping.
- **Console Flooding:** In tightly bound loops with no delays, `TRON` can significantly slow down execution and flood the console buffer. 

**Side Effects:**
- Alters the visual output of the program, which may disrupt formatted screen displays or cursor positioning (e.g., after `LOCATE`).
- Adds minor execution overhead due to the additional I/O operations required to print the trace markers.

**Behavior Constraints:**
- `TRON` and `TROFF` are strictly statements and cannot be used in expressions.
- The trace output is implicitly routed to standard output and cannot easily be redirected to a file from within the script without specific I/O redirection commands.

### 3. Code Examples

**Example 1: Basic Tracing**
```basic
10 REM Example of TRON for flow analysis
20 X = 1
30 TRON
40 IF X = 1 THEN GOTO 60
50 X = X + 1
60 PRINT "Value is: "; X
70 TROFF
80 END
```
*Expected Output:*
```
[40][60]Value is: 1
```

**Example 2: Tracing a Loop**
```basic
10 TRON
20 FOR I = 1 TO 3
30   PRINT I
40 NEXT I
50 TROFF
```
*Expected Output:*
```
[20][30] 1
[40][20][30] 2
[40][20][30] 3
[40][50]
```

### 4. Internal C-Source Mapping
- `source/parser/tron_parser.c`
- `source/virtual/debug_vm.c`
- `source/virtual/interpreter.c` (where the execution loop checks the trace flag)

### 5. Implementation Details
The `TRON` instruction is parsed by identifying the `KW_TRON` token during the lexing phase. It does not expect any subsequent expressions or arguments.

**Struct Mutations:**
- When executed, the `TRON` handler modifies the `interpreter_state` struct. Specifically, it toggles a boolean flag (e.g., `rt->trace_enabled = true`).
- The main execution loop (often a `while` loop iterating over the `AST_NODE` tree or bytecode) checks this `trace_enabled` flag before executing the next node. If true, it retrieves the `line_number` metadata from the current node and routes it to the console output buffer.

**Execution Paths:**
- Lexer: Translates the string "TRON" into `KW_TRON`.
- Parser: Creates an AST node of type `NODE_TRON`.
- Evaluator: Visits `NODE_TRON`, sets the trace flag.
- Subsequent nodes: The evaluator checks the flag and triggers `console_print_trace(node->line_number)`.

**Troubleshooting & Mutability:**
- **What can be changed:** The format of the trace output (e.g., changing `[10]` to `<Line 10>`) can be modified within the tracing output function in `debug_vm.c`.
- **What cannot be changed:** The `TRON` keyword must remain a standalone statement. It must not consume following tokens.
- **If it breaks:** If trace output stops appearing, verify that `rt->trace_enabled` is not being inadvertently reset by other commands (like `CLEAR` or `CHAIN`). Also, check if the console buffer redirection is intercepting the output. Ensure the AST nodes correctly retain their `line_number` metadata during parsing.

### 6. Cross-References / See Also
- `TROFF`
- `DEBUG`
- `PRINT`

### 7. Historical Context
`TRON` (Trace On) and `TROFF` (Trace Off) are legacy debugging commands that originated in early Microsoft BASICs, including Altair BASIC, GW-BASIC, and QBASIC. Historically, they were essential for debugging spaghetti code characterized by numerous `GOTO` statements. Our implementation maintains full backward compatibility with the GW-BASIC/QBASIC behavior while adapting the output mechanism to properly route through our modern virtual console and SDL subsystems.

### 8. Manual Testing Guide
To manually verify `TRON` functionality without the automated Python suite:
1. Open a terminal or command prompt.
2. Launch the console executable: `basicpp-console.exe`
3. Enter the following program line by line:
   `10 A = 1`
   `20 TRON`
   `30 A = A + 1`
   `40 TROFF`
   `50 PRINT A`
4. Type `RUN` and press Enter.
5. Verify that the output includes `[30][40] 2`.
6. To test immediate mode, type `TRON` and press Enter, then type `RUN` again and verify that `[10][20][30][40] 2` is printed.
7. Reset the environment by typing `NEW`.


### 9. Memory Management & Garbage Collection Profile
Under the hood, this keyword operates within the strict bounds of the BASIC++ deterministic memory manager (`core/memory.c`). When executed, any intermediate strings generated by this operation are routed to the Transient Memory Arena within the String Pool (`core/stringpool.c`). If the arena exceeds its high-water mark, an aggressive mark-and-sweep garbage collection pass is immediately triggered before the instruction completes. On embedded architectures (compiled with `-DBPP_LITE_BUILD`), this transient arena is statically clamped (default 4KB), meaning iterative loops invoking this keyword must be designed carefully to avoid `ERR_OUT_OF_MEMORY` traps. Developers porting to bare-metal systems must verify that the `memory.c` heap allocator correctly points to a continuous SRAM block without fragmentation.

### 10. Portability & Hardware Porting Concerns
Because BASIC++ is strictly C17 ISO/IEC 9899:2018 compliant, this keyword relies on zero proprietary OS APIs. When compiling for headless microcontrollers (such as the Arduino Mega or ESP32) using the `-DNO_SDL2` macro, this instruction routes all its graphical or I/O side effects through the Platform Abstraction Layer (PAL). Hardware implementers must ensure that `platform_sleep()` and `platform_get_ticks()` are properly mapped in `core/platform.c` if this keyword involves timing, yielding, or hardware-level interrupts. In cases where the underlying hardware lacks a floating-point unit (FPU), the lexer automatically maps numeric outputs to 32-bit fixed integer types if `-DBPP_NO_FLOAT` is enforced.

### 11. Abstract Syntax Tree (AST) Life Cycle
During the parsing phase, the Recursive Descent Parser encounters the token associated with this keyword. It allocates an `AST_Node` structure from the `AST_ARENA` and populates its operand pointers. At runtime, the `ast_interpreter.c` engine performs a post-order traversal to evaluate all child expression nodes before triggering the final execution hook. This two-pass system guarantees that syntax errors (like mismatched parentheses or missing commas) are caught globally before any destructive side-effects occur. Once parsed, the `AST_Node` resides in memory until `NEW` or `RUN` is executed, at which point the entire arena is zeroed out to prevent memory leaks.

### 12. C17 Standard Safety & Security Boundaries
Security and isolation are paramount. This keyword utilizes strict bounds-checking to prevent buffer overflows. Internally, any array indexing or string manipulation defaults to `size_t` for addressing, preventing negative index wraps. Stack-smashing protections are enforced virtually by the `MAX_CALL_STACK` limit defined in `config.h`. Any attempt by this keyword to access unallocated heap memory will trigger the interpreter's internal fault handler, raising a trappable BASIC error rather than causing a segmentation fault at the OS level.

### 13. Deterministic Execution & Regression Prevention
To prevent regressions across builds (Windows, Linux, or MCU), the execution of this keyword is entirely deterministic. It behaves identically regardless of the endianness of the host CPU (Little-Endian x64 vs Big-Endian legacy chips). The testing suite in `selftests_all.c` ensures that the byte-for-byte output of this operation remains identical. If a developer modifies the underlying C source code for this keyword, they MUST run the `SELFTEST` suite to verify that parsing precedence, token mapping, and garbage collection behavior have not drifted.

### 14. Performance Profiling & Optimization Rules
For developers writing performance-critical algorithms in BASIC++, be aware that calling this keyword inside a `FOR...NEXT` or `WHILE...WEND` loop incurs a minimal virtual dispatch overhead. Because the interpreter uses a switch-case dispatch engine in `exec.c`, the branch predictor on modern CPUs will optimize repeated calls. However, on 8-bit or 16-bit chips, minimizing the use of string-mutating variants of this keyword will drastically improve frame rates and execution speed.
