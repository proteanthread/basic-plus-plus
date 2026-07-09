# TCASE$

### 1. Syntax & Parameters
`TCASE$(string_expr$)`
- `string_expr$`: The string expression to be converted to title case.

### 2. Description & Usage
Converts a string to title case (sometimes referred to as proper case). `TCASE$` capitalizes the very first character of the string and any alphabetic character that immediately follows a whitespace character (like a space or tab). All other alphabetic characters in the string are forced to lowercase. Non-alphabetic characters are unaffected.

**Detailed Behavioral Edge Cases and Constraints:**
- **Implicit Truncation at 255 Bytes:** The internal `builtin_tcases` processing buffer is clamped to a static 256-byte array (`char buf[256]`). Any string whose length exceeds 255 characters will be forcibly and silently truncated. Only the first 255 characters are converted and stored into the string pool, shedding all subsequent characters.
- **Whitespace Dependency:** The parser strictly relies on the C standard library `isspace()` macro. Therefore, spaces, tabs, and newline characters act as boundaries to trigger capitalization of the next sequential alphabetical character. Non-whitespace delimiters (like hyphens or punctuation) do NOT trigger a new title case word.
- **Side Effects on Subsystems:** Evaluation inherently mutates the active statement's execution context by requesting heap allocations inside the string pool via `strpool_store`. Rapid iterative calls inside tight loops require continuous intermediate allocations in the transient memory arena, triggering aggressive garbage-collection passes if the memory ceiling is reached.
- **Null or Empty Scenarios:** Passing empty strings (`""`) or uninitialized string variables immediately yields a safely constructed zero-length string (`NULL, 0`) without touching the transformation logic. 

### 3. Code Examples
```basic
10 LET A$ = "HELLO world! 123"
20 PRINT TCASE$(A$)
30 REM Output: Hello World! 123
```

**Complex Integration Example:**
```basic
100 REM Normalizing poorly-formatted names
110 INPUT "Enter your full name: ", N$
120 N$ = TCASE$(N$)
130 PRINT "Welcome, "; N$
```

### 4. Internal C-Source Mapping
- **Lexer**: `source/lexer.h` (`KW_TCASE`), `source/lexer/lexer.c`
- **Tokenizer**: `source/core/tokenizer.c` (`TOK_TCASES`)
- **AST Node**: `source/ast.h` (`FUNC_TCASES`)
- **Parser**: `source/parser/parser_expr.c`
- **Runtime Evaluation**: `source/strings/builtins_string.c` -> `builtin_tcases()`

### 5. Implementation Details
The runtime extracts the argument and validates it as a string via `bval_is_string`. 

**In-Depth Architectural Narrative:**
- **Struct Mutations:** During AST evaluation, `eval_expr()` visits the corresponding `AST_Node` struct. The operand is retrieved into the `BValue` union structure. For the operation, a temporary localized stack buffer (`char buf[256]`) holds the payload. The length is clamped to 255. The iteration relies on a boolean state flag `after_space` initialized to `1` (true).
  - For each character `c`, if `isspace((unsigned char)c)` is true, the character is copied exactly and `after_space` is set to `1`.
  - If it is not a space, the character undergoes transformation via `after_space ? toupper(c) : tolower(c)`, and `after_space` is subsequently flipped to `0`.
  - Finally, `strpool_store(&state->strpool, buf, len)` allocates heap space.
- **Execution Paths:** 
  - *Path A (Valid String):* Length and pointer confirmed valid. Clamped to 255. Transformed utilizing the `after_space` toggle. Copied to `strpool`. Returned as `bval_string(ptr, len)`.
  - *Path B (Invalid/Empty):* Length is 0, or `data` is NULL, or type is mismatch. Instantly returns `bval_string(NULL, 0)`.
- **Error States & Fault Tolerance:** Failure points include `ERR_OUT_OF_MEMORY` if the string pool is exhausted during `strpool_store()`.
- **Modification Constraints:** 
  - *What CAN be changed:* Optimization routines inside the C-loop. The criteria for what constitutes a word boundary could be expanded from `isspace()` to also include hyphens and periods if the design requires extended Title Case rules, provided regression tests are updated.
  - *What CANNOT be changed:* The strict maximum bound of 255 bytes and the `$` suffix on the token identifier must remain rigorously enforced to prevent buffer overflows and to satisfy the `String Keyword Suffix Constraint`.

### 6. Cross-References / See Also
- `UCASE$`
- `LCASE$`
- `ICASE$`
- `MCASE$`

### 7. Historical Context
While standard GW-BASIC and QBASIC lacked native title-casing functions, BASIC++ integrates `TCASE$` to dramatically simplify data normalization processes, providing a portable, high-performance alternative to constructing complex manual string-splicing loops in user code.

### 8. Manual Testing Guide
1. Launch `basicpp-console.exe` on Windows (or `./baspp-console` on Linux).
2. Enter the following case manipulation test:
   `PRINT TCASE$("a tAle oF  TWO ciTiEs")`
3. Press **ENTER**.
4. Verify the console outputs exactly: `A Tale Of  Two Cities`
5. Test non-alphabetic characters:
   `PRINT TCASE$("jean-luc picard")`
6. Verify output: `Jean-luc Picard` (Notice that `-` does not trigger title casing because it is not a whitespace character).


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
