# REPLACE$

### 1. Syntax & Parameters
`REPLACE$(target$, search$, replace$)`
- `target$`: The original string expression to search within.
- `search$`: The substring expression to find.
- `replace$`: The substring expression to replace it with.

### 2. Description & Usage
The `REPLACE$` function searches the `target$` string for all non-overlapping occurrences of the `search$` string, replacing them with the `replace$` string, and returning the newly formed string.

### Detailed Behavioral Edge Cases and Constraints:
- **Null or Empty Target String (`target$`):** If `target$` is empty (`""`), the function returns an empty string without allocating new data.
- **Null or Empty Search String (`search$`):** If `search$` is empty (`""`), the function returns the `target$` string unchanged.
- **Null or Empty Replacement String (`replace$`):** If `replace$` is empty (`""`), all instances of the `search$` string are effectively deleted (removed) from `target$`.
- **String Length Limits:** BASIC++ imposes a maximum string length of 255 characters for the result of string operations in standard memory arenas (as per historical string length limits). If the replacement process would exceed 255 characters, the resulting string is silently truncated to exactly 255 characters.
- **Overlapping Matches:** The search is non-overlapping and processes from left to right. Once a match is found and replaced, the search resumes immediately after the end of the replaced segment.
- **Memory Allocation:** The resulting string is allocated dynamically within the active runtime's transient string pool.

### 3. Code Examples
```basic
10 PRINT REPLACE$("HELLO WORLD", "WORLD", "BASIC++") ' Output: HELLO BASIC++
20 A$ = "MISSISSIPPI"
30 PRINT REPLACE$(A$, "ISS", "X") ' Output: MXIXIPPI
40 PRINT REPLACE$("NO SPACES", " ", "") ' Output: NOSPACES
```

**Complex Integration Example:**
```basic
100 REM Cleaning up user input by replacing tabs with spaces
110 INPUT "Enter data: ", RAW$
120 CLEAN$ = REPLACE$(RAW$, CHR$(9), " ")
130 CLEAN$ = REPLACE$(CLEAN$, "  ", " ") ' Reduce double spaces to single spaces
140 PRINT "Cleaned: "; CLEAN$
```

### 4. Internal C-Source Mapping
- **Lexer**: `source/parser/lexer.c`
- **AST Node**: `source/parser/ast.c`
- **Function Registry**: `source/functions/builtins.c`
- **Runtime Evaluation**: `source/strings/builtins_string.c -> builtin_replaces()`

### 5. Implementation Details
The runtime extracts the arguments, ensuring all three are valid string values (`bval_is_string`). If any argument fails string validation, an empty string is returned. The engine iterates through the target string character by character. When a match matching the exact sequence of `search$` is identified using `memcmp`, the `replace$` string is injected into the output buffer, provided it does not exceed the 255-character hard limit. 

### In-Depth Architectural Narrative:
- **Struct Mutations:** During evaluation, `builtin_replaces()` creates a local stack buffer (`char buf[256]`). The length is continuously bounds-checked (`if (j + repl_len <= 255)`). Memory is safely committed to the persistent execution cycle via `strpool_store(&state->strpool, buf, j)`. The resulting pointer is returned encapsulated in a `BValue` struct indicating `VAL_STRING`.
- **Error States & Fault Tolerance:** 
  - If a non-string argument is provided, the function gracefully degrades, returning an empty `BValue` string (`bval_string(NULL, 0)`) without raising a fatal type error, though `error_raise` might be bubbled up during AST evaluation if argument types are fundamentally mismatched.
  - No `ERR_OUT_OF_BOUNDS` is thrown for string length excesses; instead, the string is truncated safely to 255 characters.
- **Modification Constraints:** 
  - *What CAN be changed:* Optimization of the substring search algorithm (e.g., using Boyer-Moore instead of a naive byte-by-byte scan) can be implemented if performance bottlenecks are identified.
  - *What CANNOT be changed:* The 255-character maximum output buffer size is deeply entrenched in string pool constants and legacy compatibility expectations; removing it would require an overhaul of `strpool_store` and legacy dialect constraints.

### 6. Cross-References / See Also
- `INSTR`
- `MID$`
- `EDIT$`

### 7. Historical Context
The `REPLACE$` function was not present in the original IBM GW-BASIC or QuickBASIC implementations. It is a modern extension adopted from Visual Basic and other later BASIC dialects to facilitate string manipulation without resorting to complex loops using `INSTR`, `LEFT$`, `MID$`, and `RIGHT$`.

### 8. Manual Testing Guide
1. Launch `basicpp-console.exe`.
2. Execute direct mode command: `PRINT REPLACE$("ABC-DEF-ABC", "ABC", "123")` and press ENTER.
3. Verify the output matches `123-DEF-123`.
4. Test empty replace: `PRINT REPLACE$("hello world", "l", "")` to verify output is `heo word`.


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
