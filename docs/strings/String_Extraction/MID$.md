# MID$

## 1. Syntax & Parameters

```text
MID$(string_expr$, start_expr [, length_expr])
```

* **string_expr$**: A valid string expression from which the substring will be extracted.
* **start_expr**: A numeric expression indicating the 1-based starting position within `string_expr$` from which extraction should begin.
* **length_expr** *(Optional)*: A numeric expression defining the exact number of characters to extract. If omitted, `MID$` intrinsically extracts all remaining characters from `start_expr` to the absolute end of the string.

## 2. Description & Usage

The `MID$` function is the most versatile string extraction tool in the BASIC language. It allows the extraction of a precise subsection from the interior of any given string, defined by a 1-based physical offset and an optional constraint on length. 

**Behavioral Edge Cases & Execution Paths:**
* **Omitted Length:** If `length_expr` is absent, the execution path implicitly assumes `MAX_STRING_LENGTH`, guaranteeing extraction proceeds cleanly to the terminus of the source string without artificially breaking due to undeclared bounds.
* **Start Exceeds Length:** If `start_expr` is larger than the total length of `string_expr$`, the parser will immediately yield an empty string `""`, avoiding invalid memory offset traps.
* **Zero or Negative Start:** Classic BASIC operates on a strictly 1-based indexing model. If `start_expr` falls to 0 or becomes negative, the underlying engine enforces a hard baseline of 1 to ensure safe evaluation rather than an "Illegal function call".
* **Negative Length:** Similar to `LEFT$` and `RIGHT$`, supplying a negative `length_expr` defaults to a safe-mode return of an empty string `""`.
* **Length Overshoot:** If `start_expr` is valid, but `length_expr` demands more characters than are mathematically remaining in the string, the function elegantly truncates the operation precisely at the string's actual boundary.

## 3. Code Examples

**Example 1: Standard 3-Argument Extraction**
```basic
10 LET A$ = "ABCDEFGHIJ"
20 PRINT MID$(A$, 4, 3)
```
*Output:* `DEF`

**Example 2: 2-Argument Extraction (To the End)**
```basic
10 LET B$ = "123456789"
20 PRINT MID$(B$, 6)
```
*Output:* `6789`

**Example 3: Graceful Overshoot Boundary**
```basic
10 LET C$ = "SHORT"
20 PRINT MID$(C$, 2, 50)
```
*Output:* `HORT`

**Example 4: Start Index Beyond Length**
```basic
10 LET D$ = "DATA"
20 PRINT "[" + MID$(D$, 10, 2) + "]"
```
*Output:* `[]`

## 4. Internal C-Source Mapping

* **Entry Point / Lexing:** Recognized as a string extraction function handling optional parameters.
* **Parser Registration:** Governed inside the built-in function registry via `FCAT_STRING`.
* **Wrapper Implementation:** `builtin_mid` strategically deployed in `source/strings/builtins_string.c`.
* **Core Logic Engine:** `bval_mid` heavily utilized within `source/core/value.c`.
* **String Allocation:** Interacts exclusively with `strpool_alloc` (`source/core/stringpool.c`).

## 5. Implementation Details

**Execution Pipeline & Memory Operations:**
1. **Polymorphic Argument Checking:** The wrapper function `builtin_mid` evaluates parameter counts. If `argc < 3`, the missing `length_expr` is natively synthesized by creating a proxy `BValue` loaded with `MAX_STRING_LENGTH`. This avoids generating secondary legacy functions for 2-argument processing.
2. **Indexing Shift:** Once the parameters reach `bval_mid` in `source/core/value.c`, the 1-based BASIC indexing is surgically shifted to 0-based C indexing via `pos = (int)bval_to_int(start_val) - 1`.
3. **Rigid Bounds Computation:** Boundary sanitization is intense:
   - If `pos < 0` (due to user providing 1, 0, or negative), `pos` is coerced to `0`.
   - If `pos > s->v.sval.length`, it clamps `pos` explicitly to `s->v.sval.length`.
   - If `count < 0`, it defaults `count = 0`.
   - To prevent buffer reads beyond allocated memory, if `pos + count > s->v.sval.length`, `count` is geometrically reduced: `count = s->v.sval.length - pos;`.
4. **Pool Instantiation:** `strpool_alloc` reserves exactly `count` bytes. Should the pool face exhaustion, an `ERR_SORRY` prevents pointer anomalies.
5. **Memory Slicing:** A targeted `memcpy` streams exactly `count` bytes from the exact internal offset `s->v.sval.data + pos` to the pristine pool segment. The resulting length is stored explicitly in the returned `BValue` bypassing the need for a null terminator.

**What to do if it breaks:**
If `MID$` truncates inappropriately or begins throwing segmentation faults under heavy load, check `pos = (int)bval_to_int(start_val) - 1;` logic. Specifically, ascertain if extremely large unsigned integers sent via floating-point arguments are overflowing `pos` into negative C boundaries, bypassing subsequent constraint checks.

## 6. Cross-References / See Also

* `LEFT$` - Extracts characters starting from the left.
* `RIGHT$` - Extracts characters rooted at the right limit.
* `SEG$` - Extracts based on absolute start and end indices.
* `INSTR` - Finds the 1-based index required to feed into `MID$`.
* `REPLACES$` - Replaces substrings across the string data.

## 7. Historical Context

In legacy frameworks, `MID$` was famously hostile towards zero or negative arguments, aggressively halting the machine state via Error 5. The modern architecture in BASIC++ intercepts mathematically erratic bounds gracefully without compromising determinism. Furthermore, some legacy parsers enforced complex parameter matrices for handling the 2-argument variant, whereas BASIC++ dynamically populates the omitted length directly at the wrapper level (`MAX_STRING_LENGTH`), maintaining extremely tight execution efficiency.

## 8. Manual Testing Guide

Use the terminal interface to confirm `MID$` handles both optimal and aberrant bounding requests correctly:

1. **Launch the Interface:**
   - On Windows: Run `basicpp-console.exe`
   - On Linux: Run `./baspp-console`

2. **Execute Boundary Constraints (Interactive Mode):**
   ```basic
   PRINT MID$("INTERPRETER", 6, 3)
   ```
   *Expect:* `PRE`

   ```basic
   PRINT MID$("INTERPRETER", 6)
   ```
   *Expect:* `PRETER` (Implicit length to the end)

   ```basic
   PRINT MID$("INTERPRETER", 0, 5)
   ```
   *Expect:* `INTER` (0 coerces safely to the 1st position)

   ```basic
   PRINT MID$("INTERPRETER", -5, 5)
   ```
   *Expect:* `INTER` (Negative coerces safely to the 1st position)

   ```basic
   PRINT MID$("INTERPRETER", 100, 5)
   ```
   *Expect:* A blank line (Start value overshoots string length safely)

   ```basic
   PRINT MID$("INTERPRETER", 1, -10)
   ```
   *Expect:* A blank line (Negative length safe trap)


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
