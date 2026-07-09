# RIGHT$

## 1. Syntax & Parameters

```text
RIGHT$(string_expr$, length_expr)
```

* **string_expr$**: A valid string expression from which the rightmost characters will be extracted. Must evaluate to a string.
* **length_expr**: A numeric expression specifying the number of characters to extract from the right side of `string_expr$`. Must evaluate to a numeric value. Fractional values will be implicitly truncated or rounded to an integer before evaluation.

## 2. Description & Usage

The `RIGHT$` keyword is a core string manipulation function designed to extract a precise number of characters from the absolute right (the end) of a target string. It operates in constant harmony with `LEFT$`, allowing users to parse suffixes, file extensions, and terminal control sequences.

**Behavioral Edge Cases & Execution Paths:**
* **Length Exceeds Source:** If `length_expr` computes to a number greater than or equal to `LEN(string_expr$)`, the function will return the entire `string_expr$` wholesale. It will not generate an overflow error.
* **Zero Length:** If `length_expr` equates exactly to 0, `RIGHT$` immediately yields a zero-length (empty) string `""`.
* **Negative Length:** In alignment with our fail-safe boundary policies, if `length_expr` resolves to a negative integer, the subsystem interprets it as 0, safely projecting an empty string `""` without halting execution.
* **Empty Source String:** If the original `string_expr$` is empty or uninitialized, any requested length will securely return an empty string.

## 3. Code Examples

**Example 1: Basic Extraction**
```basic
10 LET FILE$ = "REPORT.TXT"
20 PRINT RIGHT$(FILE$, 4)
```
*Output:* `.TXT`

**Example 2: Over-fetching Gracefully**
```basic
10 LET WORD$ = "CAT"
20 PRINT RIGHT$(WORD$, 10)
```
*Output:* `CAT`

**Example 3: Safe Parsing of Variables**
```basic
10 LET INPUT$ = "ERROR CODE: 404"
20 LET L = LEN(INPUT$)
30 PRINT RIGHT$(INPUT$, 3)
```
*Output:* `404`

**Example 4: Negative Boundaries**
```basic
10 LET DATA$ = "SECURE"
20 PRINT "[" + RIGHT$(DATA$, -9) + "]"
```
*Output:* 
`[]`

## 4. Internal C-Source Mapping

* **Entry Point / Lexing:** The keyword `RIGHT$` is tokenized as a standard string-returning function.
* **Parser Registration:** Mounted inside the built-in function registry via `FCAT_STRING`.
* **Wrapper Implementation:** `builtin_right` located within `source/strings/builtins_string.c`.
* **Core Logic Engine:** `bval_right` inside `source/core/value.c`.
* **String Allocation:** Memory is provisioned via `strpool_alloc` natively found in `source/core/stringpool.c`.

## 5. Implementation Details

**Execution Pipeline & Memory Operations:**
1. **Delegation & Validation:** The interpreter routes `RIGHT$` calls through the wrapper `builtin_right`, directly injecting the parameters into `bval_right`. Type integrity checks immediately assert that the primary parameter is indeed a `VAL_STRING` and the secondary is inherently numeric. Failing these type assertions yields `error_raise(ERR_WHAT, line_num)` and defaults the return to an empty string to prevent downstream cascading failures.
2. **Bounds Assessment & Pointer Math:** The requested extraction length is resolved into a 32-bit integer. If `count < 0`, the logic automatically zeroes it. If `count > s->v.sval.length`, it is rigorously clamped to match the total source length. The extraction pointer calculation is highly deliberate: `start = s->v.sval.length - count;`. This securely targets the origin offset from which the rightmost chunk begins.
3. **Pool Allocation:** The `StringPool` assigns a continuous memory space of exactly `count` size. Should memory constraints result in a `NULL` allocation pointer, an `ERR_SORRY` (Memory Exhaustion) is flagged, gracefully terminating the assignment.
4. **Data Duplication:** A precise `memcpy` operation copies data from the source offset `(s->v.sval.data + start)` directly to the newly instanced pool node. The destination is then bound to a `BValue` struct returning to the execution flow.

**What to do if it breaks:**
Should `RIGHT$` unexpectedly yield characters seemingly offset by 1 or displaying internal memory fragments, suspect an issue within the core calculation: `start = s->v.sval.length - count`. Verify via debugger that `s->v.sval.length` accurately mirrors the byte length of the string and does not inadvertently count null terminators.

## 6. Cross-References / See Also

* `LEFT$` - Extracts characters from the left side of a string.
* `MID$` - Extracts characters from a middle section of a string.
* `SEG$` - Extracts a segment of a string defined by start and end positions.
* `LEN` - Obtains the size of the target string.
* `INSTR` - Discovers a specific pattern's position inside a string.

## 7. Historical Context

Historically within MS-DOS lineages like GW-BASIC, passing a negative number to `RIGHT$` inherently provoked a terminal Error 5 (Illegal function call). By pivoting to a fail-safe paradigm, BASIC++ shields long-running background tasks from violently aborting due to negative computations cascading into a string function. Furthermore, the robust dynamic memory pool of BASIC++ permits the truncation and extraction of strings far surpassing the legacy 255-byte limit historically shackled to 8-bit string descriptors.

## 8. Manual Testing Guide

Ensure the functional integrity of `RIGHT$` by running manual diagnostics directly within the terminal interface:

1. **Launch the Interface:**
   - On Windows: Run `basicpp-console.exe`
   - On Linux: Run `./baspp-console`

2. **Execute Boundary Constraints (Interactive Mode):**
   ```basic
   PRINT RIGHT$("DEVELOPMENT", 4)
   ```
   *Expect:* `MENT`

   ```basic
   PRINT RIGHT$("DEVELOPMENT", 50)
   ```
   *Expect:* `DEVELOPMENT` (Demonstrates bounding without memory overflow)

   ```basic
   PRINT RIGHT$("DEVELOPMENT", 0)
   ```
   *Expect:* A blank line (Zero-length response).

   ```basic
   PRINT RIGHT$("DEVELOPMENT", -12)
   ```
   *Expect:* A blank line, proving that negative length values are safely intercepted.

3. **Verify Type-Safety Error Trapping:**
   ```basic
   PRINT RIGHT$(999, 2)
   ```
   *Expect:* A `Type mismatch` or `WHAT?` error.


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
