# RUN

## 1. Syntax & Parameters
`RUN [line_number]`
`RUN "filename"[, R]`
- `line_number` (Optional): The line number at which to begin execution. If omitted, execution starts at the lowest line number.
- `"filename"` (Optional): A string expression indicating a file to load and execute.
- `R` (Optional): A flag indicating that all open data files should remain open during the RUN (only applicable when loading a new file).

## 2. Description & Usage
The `RUN` command is the primary trigger for executing a BASIC++ program. 
When used without arguments, it clears all variables from memory (simulating a `CLEAR` command), resets all internal pointers (like `RESTORE`), and begins execution at the first line of the program. If a line number is specified, execution begins at that line, but variables and pointers are still cleared.
When a string filename is provided, `RUN` clears the current workspace, loads the specified program from the filesystem into memory, and immediately begins execution.

**Edge Cases & Boundary Conditions:**
- **Type Coercion and Overflows:** A line number passed to `RUN` must be a valid 32-bit unsigned integer. Negative line numbers or numbers exceeding `MAX_LINE_NUM` will result in an `ERR_UNDEFINED_LINE_NUMBER` or `ERR_SYNTAX_ERROR`.
- **Side Effects on Subsystems:** Evaluation inherently mutates the active statement's execution context. A `RUN` command triggers an immediate global reset of the `symtab_t` (symbol table), wiping all variable states unless executing via `CHAIN` or maintaining files with the `R` flag.
- **Missing File Scenarios:** If `RUN "file.bas"` is called and the file does not exist, an `ERR_FILE_NOT_FOUND` is thrown, and the current workspace remains intact.
- **Execution Paths:** Executing `RUN` from within a running program script is permitted and acts as a self-restart or a load-and-execute chain, destroying the current execution stack.

## 3. Code Examples
**Example 1: Standard Execution**
```basic
10 PRINT "System booting..."
20 X = 5
RUN
' Output: System booting... (X is reset to 0 internally)
```

**Example 2: Execution from a Specific Line**
```basic
10 PRINT "A"
20 PRINT "B"
RUN 20
' Output: B
```

**Example 3: Loading and Running a File**
```basic
RUN "game.bas"
```

**Complex Syntax Combinations:**
```basic
10 IF LEVEL = 2 THEN RUN "level2.bas", R
```

## 4. Internal C-Source Mapping
- **Lexer**: `source/parser/lexer.c` (`KW_RUN`)
- **AST Node**: `source/parser/ast.c` (`STMT_RUN`)
- **Runtime Execution**: `source/progmgmt/parser_progmgmt.c -> pi_parse_run()`

## 5. Implementation Details
The runtime parser evaluates the instruction and delegates to the interpreter state engine. 
When `pi_parse_run()` is invoked, it first checks for arguments. If a line number is passed, it searches the `AST_Program` linked list for the target node. 
Before execution begins, `RUN` calls `symtab_clear()` to wipe the variable pool, resets the `data_ptr` to the first `DATA` statement, and clears the call stack. If the `R` flag is absent during a file load, it iterates through `file_channels` calling `platform_file_close()`. The execution pointer `interp->ip` is then set to the target AST node, and the main evaluation loop `interp_run_loop()` is engaged.

**Struct Mutations:**
The `InterpreterState` struct undergoes a massive reset. `call_stack_ptr = 0`, `data_ptr = NULL`, and the `symtab` hash map is truncated and reallocated. If a filename is provided, the current `AST_Program` is completely deallocated via `ast_free_program()`, and a new AST is constructed by the parser from the loaded file buffer.

**Troubleshooting Scenarios & Error States:**
- **ERR_UNDEFINED_LINE_NUMBER:** Thrown if `RUN 500` is called and line 500 does not exist.
- **ERR_OUT_OF_MEMORY:** Thrown if `RUN "file.bas"` is executed and the loaded file exceeds available RAM, or the AST construction exhausts the node allocator pool.
- **Modification Constraints:** 
  - *What CAN be changed:* The AST deallocation/reallocation routines during a file load can be optimized for memory pooling.
  - *What CANNOT be changed:* The standard behavior of clearing variables upon a local `RUN` must be preserved for GW-BASIC parity. Do not change `RUN` to behave like `GOTO`.

## 6. Cross-References / See Also
GOTO, CONT, CLEAR, CHAIN, LOAD

## 7. Historical Context
GW-BASIC / QBASIC compatible structural management commands. The behavior of clearing variables upon execution is a hallmark of early microcomputer BASICs, ensuring a clean slate.

## 8. Manual Testing Guide
1. Launch `basicpp-console.exe` from your terminal.
2. Type: `A = 50`
3. Type: `10 PRINT A`
4. Type: `20 A = 99`
5. Type: `RUN` and press ENTER. 
6. Verify it prints `0` for line 10 (because RUN cleared the A=50 set in immediate mode before starting).
7. Type `PRINT A`. Verify it prints `99` (because variables persist after execution finishes).


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
