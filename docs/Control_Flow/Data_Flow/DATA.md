# DATA

### 1. Syntax & Parameters
`DATA <constant_1> [, <constant_2>, ..., <constant_n>]`

*   **`constant`**: A numeric literal (integer or floating-point) or a string literal. Unquoted strings are permitted in traditional BASIC, provided they do not contain commas or significant leading/trailing whitespace. If a string contains a comma or requires leading spaces, it must be enclosed in double quotes (e.g., `"Hello, World"`).

### 2. Description & Usage
The `DATA` statement is a non-executable declarative command used to store a sequence of constants within a BASIC++ program. These constants are sequentially accessed and assigned to variables at runtime by the `READ` statement. 

**Usage:**
`DATA` statements can be placed anywhere in a program. Before execution begins, the BASIC++ interpreter scans the entire AST (Abstract Syntax Tree) to build a global, sequential internal list of all `DATA` constants in the exact order they appear, from the lowest line number to the highest. 

This mechanism is primarily utilized for loading static configuration, pre-computed tables (like sine/cosine tables or sprite graphics data), level maps for games, or batch initialization of arrays without relying on external file I/O operations.

**Edge Cases & Boundary Conditions:**
*   **Multiple DATA Statements:** When multiple `DATA` lines exist, their contents are concatenated logically into a single continuous stream.
*   **Trailing Commas:** A trailing comma (e.g., `10 DATA 1, 2,`) is strictly illegal and will generate an `ERR_SYNTAX` during parsing.
*   **Empty DATA Statements:** A `DATA` keyword without any arguments is ignored but legal syntax.
*   **Type Mismatch:** `DATA` does not enforce type-checking until a `READ` statement attempts to pull the value. If a string constant is read into a numeric variable, it will trigger an `ERR_TYPE_MISMATCH` at runtime.

### 3. Code Examples

**Basic Numeric and String Data:**
```basic
10 READ A, B$, C
20 PRINT "ID: "; A; " Name: "; B$; " Age: "; C
30 DATA 101, "Alice Smith", 28
```

**Using Multiple DATA Statements for Arrays:**
```basic
10 DIM MAP(2, 2)
20 FOR Y = 0 TO 2
30   FOR X = 0 TO 2
40     READ MAP(X, Y)
50   NEXT X
60 NEXT Y
70 DATA 1, 0, 1
80 DATA 0, 1, 0
90 DATA 1, 0, 1
```

**Unquoted vs. Quoted Strings:**
```basic
10 READ W1$, W2$, W3$
20 PRINT W1$, W2$, W3$
30 DATA HELLO, "WORLD, WIDE", WEB
```

### 4. Internal C-Source Mapping
*   `source/parser/parser.c`
*   `source/runtime/runtime.c`
*   `source/virtual/vm.c`
*   `include/lexer.h`

### 5. Implementation Details
The `DATA` instruction is identified in the Lexer by the `KW_DATA` token. 

**Parser Expectations:**
Because `DATA` is declarative, the parser's main job is to tokenize the constants and bind them to the AST. It ensures comma-separation and captures the exact literal values. In BASIC++, the interpreter performs a pre-execution pass (`runtime.c`) to traverse the AST, locate all `AST_DATA` nodes, and build a linear queue or linked list of data pointers representing the global data stream.

**Struct Mutations:**
*   **Global Data Stream:** The runtime environment (`rt_state *rt`) contains a `data_head` and `data_tail` pointer linking to the collected `DATA` constants.
*   **Data Pointer (`rt->data_ptr`):** Initialized to point to the first element of this global stream when execution begins or when `RESTORE` is called.

**Execution Paths & Error Handling:**
1. During the pre-execution AST scan, if a `KW_DATA` node is found, its arguments are appended to the VM's global data list.
2. During actual execution, if the VM's program counter (`pc`) lands on a `DATA` statement, it treats it as a `NOP` (No-Operation) and immediately advances to the next instruction, as the data has already been collected.
3. Syntax errors, such as missing commas between constants, are caught during the initial parse pass, throwing `ERR_SYNTAX`.

**What to do if it breaks:**
If `DATA` values are skipping or being read incorrectly, inspect the AST construction phase in `parser.c`. Ensure that unquoted strings aren't accidentally swallowing adjacent commas. Verify the pre-execution pass in `runtime.c` is correctly chaining the `DATA` nodes in ascending line number order.

### 6. Cross-References / See Also
*   [`READ`](READ.md)
*   [`RESTORE`](RESTORE.md)

### 7. Historical Context
The `DATA`, `READ`, and `RESTORE` triumvirate is one of the oldest concepts in the BASIC language, originating from the 1964 Dartmouth BASIC. It provided a simple way to hardcode data before the widespread availability of file systems. BASIC++ preserves the exact GW-BASIC / QBASIC and ECMA-116 semantics for `DATA`, including the quirk of allowing unquoted string literals.

### 8. Manual Testing Guide
1. Launch `basicpp-console.exe` (Windows) or `baspp-console` (Linux).
2. To test basic assignment, type:
   ```
   10 READ X, Y$
   20 PRINT X; Y$
   30 DATA 99, BALLOONS
   ```
   Type `RUN`. Output must be `99 BALLOONS`.
3. To test unquoted string handling with commas, type:
   ```
   10 READ A$
   20 PRINT A$
   30 DATA "HELLO, THERE"
   ```
   Type `RUN`. Output must be `HELLO, THERE`.
4. To test syntax validation, type:
   ```
   10 DATA 1, 2,
   ```
   Type `RUN`. Ensure it throws an `ERR_SYNTAX` during parsing due to the trailing comma.


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
