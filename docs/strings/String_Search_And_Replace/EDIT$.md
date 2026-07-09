# EDIT$

### 1. Syntax & Parameters
`EDIT$(target$, code)`
- `target$`: The original string expression to format.
- `code`: An integer bitmask specifying the formatting operations to perform.

### 2. Description & Usage
The `EDIT$` function reformats a string based on the `code` argument. `code` acts as a bitmask where multiple formatting options can be combined by adding their values together (or using bitwise `OR`).

### Bitmask Values for `code`:
- **1:** Discard the parity bit (masks each character to 7-bit ASCII by applying a bitwise `AND 127`).
- **2:** Discard all spaces (` `) and tabs (`\t`).
- **4:** Discard all carriage returns (`\r`), line feeds (`\n`), form feeds (`\f`), backspaces (`\b`), escapes (`ESC` / 27), and nulls (`\0`).
- **8:** Remove leading spaces and tabs (left trim).
- **16:** Compress multiple consecutive spaces or tabs into a single space.
- **32:** Convert all lowercase letters to uppercase.
- **64:** Convert all square brackets (`[` and `]`) to parentheses (`(` and `)`).
- **128:** Remove trailing spaces and tabs (right trim).
- **256:** Preserve all characters contained within double quotes (`"`). When set, any formatting flags are ignored for substrings enclosed in double quotes.

### Detailed Behavioral Edge Cases and Constraints:
- **Combining Flags:** By adding flags together (e.g., `8 + 128 = 136`), multiple formats can be applied simultaneously (in this case, trimming both left and right edges).
- **Order of Operations:** The internal engine processes characters left-to-right in a single pass. Bitwise masking (flag 1) and uppercase conversion (flag 32) occur before discard flags. Trimming (flags 8 and 128) is executed on the final assembled buffer after all other character evaluations.
- **Quote Preservation (Flag 256):** If flag 256 is active, the engine toggles a state machine when encountering a `"` character. All subsequent characters are passed perfectly intact until the next `"` character is reached.
- **Type Coercion:** If `code` evaluates to a non-numeric type, a type mismatch error is raised. 
- **Memory Allocation:** The result is dynamically allocated in the string pool. The memory allocation scales strictly to the output size.

### 3. Code Examples
```basic
10 S$ = "   hello   [world]  "
20 PRINT ">" + EDIT$(S$, 8 + 128) + "<" ' Outputs: >hello   [world]< (Trims spaces)
30 PRINT EDIT$(S$, 32 + 64) ' Outputs:    HELLO   (WORLD)  (Uppercase & Brackets)
40 PRINT EDIT$(S$, 2) ' Outputs: hello[world] (Removes all spaces)
```

**Complex Integration Example:**
```basic
100 REM Cleaning messy data input
110 RAW$ = "   ID001   " + CHR$(9) + "   [ACTIVE]   "
120 REM Compress spaces (16), Trim edges (136), Uppercase (32), Brackets to parens (64)
130 CLEAN$ = EDIT$(RAW$, 16 + 136 + 32 + 64)
140 PRINT CLEAN$ ' Output: ID001 (ACTIVE)
```

### 4. Internal C-Source Mapping
- **Lexer**: `source/parser/lexer.c`
- **AST Node**: `source/parser/ast.c`
- **Function Registry**: `source/functions/builtins.c`
- **Runtime Evaluation**: `source/strings/builtins_string.c -> builtin_edit()`

### 5. Implementation Details
The runtime extracts the string and numeric code arguments. It allocates a temporary heap buffer (`malloc`) equal to the original string length plus one, guaranteeing sufficient memory even if no characters are discarded. It evaluates each character linearly. 

### In-Depth Architectural Narrative:
- **Struct Mutations:** During processing, the active flag bits are queried via bitwise `AND` (e.g., `if (code & 256)`). The filtered output is built inside the allocated heap array. Afterwards, boundary pointers (`start`, `end`) are adjusted to satisfy trimming flags (8 and 128). Finally, the precise substring is cloned into the interpreter's string pool (`strpool_alloc`), and the temporary heap buffer is explicitly freed (`free(dest)`).
- **Error States & Fault Tolerance:** 
  - `ERR_WHAT`: Raised if the argument count is incorrect or if types mismatch (e.g., target is not a string, or code is not numeric).
  - `ERR_SORRY`: Raised if dynamic heap allocation (`malloc`) for the temporary buffer fails due to system exhaustion.
- **Modification Constraints:** 
  - *What CAN be changed:* New bitmask flags can be allocated for unused bits (e.g., 512) for future formatting rules.
  - *What CANNOT be changed:* The execution order of flags 8 and 128 (which must execute after the main character loop) must remain intact.

### 6. Cross-References / See Also
- `TRIM$`
- `LTRIM$`
- `RTRIM$`
- `UCASE$`
- `LCASE$`

### 7. Historical Context
The `EDIT$` function is derived from Digital Equipment Corporation (DEC) BASIC (such as BASIC-PLUS) and True BASIC. It was traditionally used as an extremely powerful monolithic string formatter before individual functions like `TRIM$` or `UCASE$` became standardized in Microsoft dialects.

### 8. Manual Testing Guide
1. Launch `basicpp-console.exe`.
2. Test uppercase and bracket replacement: `PRINT EDIT$("this is a [test]", 32 + 64)` -> Output: `THIS IS A (TEST)`.
3. Test space compression and quotes: `PRINT EDIT$("a   b   c   ""d   e""", 16 + 256)` -> Output: `a b c "d   e"`.


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
