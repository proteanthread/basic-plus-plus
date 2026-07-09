# SEG$

## 1. Syntax & Parameters

```text
SEG$(string_expr$, start_expr, end_expr)
```

* **string_expr$**: A valid string expression from which a segment will be isolated.
* **start_expr**: A numeric expression representing the 1-based index denoting where the extraction begins (inclusive).
* **end_expr**: A numeric expression representing the 1-based index denoting where the extraction concludes (inclusive).

## 2. Description & Usage

The `SEG$` function serves as a specialized bounds-based extraction mechanism, differentiating itself from `MID$` by requiring an absolute terminating boundary rather than a relative character count. This design intrinsically supports operations where substring boundaries are derived from two precise positional markers (such as output from dual `INSTR` calls).

**Behavioral Edge Cases & Execution Paths:**
* **End Exceeds Length:** If `end_expr` maps to an index exceeding the actual string dimension `LEN(string_expr$)`, the logic strictly clamps `end_expr` to the absolute maximum string length.
* **Start Exceeds End:** If the evaluated `start_expr` surpasses the evaluated `end_expr` (an inverted bound scenario), the execution will securely yield a zero-length empty string `""`, recognizing the mathematical impossibility of the request.
* **Negative or Zero Bounds:** The system rigorously guards 1-based indices. Thus, a `start_expr` that resolves mathematically below 1 will be transparently corrected to exactly 1.
* **Start Exceeds Source Length:** If the `start_expr` lies entirely outside the source string bounds, the function generates an empty string `""`.

## 3. Code Examples

**Example 1: Basic Segment Isolation**
```basic
10 LET A$ = "DICTIONARY"
20 PRINT SEG$(A$, 4, 7)
```
*Output:* `TION`

**Example 2: Integration with INSTR boundaries**
```basic
10 LET XML$ = "<TAG>DATA</TAG>"
20 LET S = INSTR(XML$, ">") + 1
30 LET E = INSTR(S, XML$, "<") - 1
40 PRINT SEG$(XML$, S, E)
```
*Output:* `DATA`

**Example 3: End Parameter Clamping**
```basic
10 LET B$ = "LOGICAL"
20 PRINT SEG$(B$, 3, 500)
```
*Output:* `GICAL`

**Example 4: Inverted Boundaries**
```basic
10 LET C$ = "OVERLAP"
20 PRINT "[" + SEG$(C$, 5, 2) + "]"
```
*Output:* `[]`

## 4. Internal C-Source Mapping

* **Entry Point / Lexing:** Processed alongside string functions requiring ternary numeric evaluation.
* **Parser Registration:** Tracked within the built-in function registry via `FCAT_STRING`.
* **Direct Implementation:** Unlike other core extraction tools, `SEG$` operates natively within `builtin_segs` housed inside `source/strings/builtins_string.c` directly manipulating the `BValue` constructs.

## 5. Implementation Details

**Execution Pipeline & Memory Operations:**
1. **Argument Population Checks:** The C-level function `builtin_segs` first asserts parameter counts (`argc < 3`). If absent, it immediately forces an empty string. Type validity evaluates `string_expr$` explicitly against `bval_is_string`.
2. **Bounds Assessment & Struct Access:** `start` and `end` targets are converted into pure 32-bit `int`. The source string length is dynamically extracted directly from the struct memory `args[0].v.sval.length`.
3. **Boundary Sanitization Layer:** The logic strictly protects the structural integrity of memory bounds:
   - `if (start < 1) start = 1;` enforces the 1-based anchor.
   - `if (end > slen) end = slen;` ensures the extraction does not traverse out-of-bounds pointer territory.
   - `if (start > end || start > slen || s == NULL || slen == 0)` intercepts inverted logic, overshoots, and unallocated strings, instantly yielding a null string to preserve host stability.
4. **Length Mathematical Calculation:** The final dimension of the requested slice is computed via `out_len = end - start + 1`. This accounts for the inclusive nature of the boundary targets.
5. **Memory Slicing Strategy:** Instead of standard pool allocations, it directly utilizes `strpool_store`, pointing statically to the physical offset `s + start - 1`, and instructing the `StringPool` to clone precisely `out_len` bytes.

**What to do if it breaks:**
If `SEG$` causes the interpreter to return spurious terminal errors or segmentation issues during massive parsing tasks, verify that the `out_len` logic strictly adheres to bounds. Should 64-bit platforms incorrectly coerce fractional floats during `bval_to_int`, negative intervals may slip past if not explicitly cast. Debug inside `builtin_segs` and monitor the `start` vs `end` variable traps.

## 6. Cross-References / See Also

* `MID$` - Extracts based on a start index and length count.
* `LEFT$` - Extracts characters from the absolute beginning.
* `RIGHT$` - Extracts characters from the absolute end.
* `INSTR` - Commonly used to locate dynamic bounds for `SEG$`.
* `REPLACES$` - Used for finding and substituting string content.

## 7. Historical Context

The `SEG$` keyword is historically prevalent in specific specialized BASIC dialects (like certain variants of HP Time-Shared BASIC or specialized text-processing engines). Legacy MS-DOS interpreters (GW-BASIC, QBASIC) relied entirely on `MID$` for arbitrary extractions. The inclusion of `SEG$` in BASIC++ provides modern users with superior syntactic sugar for bounded text parsing (like JSON, XML, or binary header isolation) natively eliminating the need to manually compute string lengths mathematically before invoking a `MID$` slice.

## 8. Manual Testing Guide

Validate `SEG$` bounds safety manually using the terminal command interface:

1. **Launch the Interface:**
   - On Windows: Run `basicpp-console.exe`
   - On Linux: Run `./baspp-console`

2. **Execute Boundary Constraints (Interactive Mode):**
   ```basic
   PRINT SEG$("EXTRACTOR", 3, 7)
   ```
   *Expect:* `TRACT`

   ```basic
   PRINT SEG$("EXTRACTOR", 3, 99)
   ```
   *Expect:* `TRACTOR` (End constraint clamped cleanly)

   ```basic
   PRINT SEG$("EXTRACTOR", 0, 4)
   ```
   *Expect:* `EXTR` (Start constraint bounded up to 1)

   ```basic
   PRINT SEG$("EXTRACTOR", 8, 4)
   ```
   *Expect:* A blank line (Inverted parameters safely trapped)

   ```basic
   PRINT SEG$("EXTRACTOR", 20, 25)
   ```
   *Expect:* A blank line (Start point beyond source length limits trapped)


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
