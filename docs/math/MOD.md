# MOD

## 1. Syntax & Parameters

**Syntax:**
`result = expression1 MOD expression2`

**Parameters:**
*   **`expression1`**: The dividend. Any valid numeric expression. If it is a floating-point value, it is first truncated to an integer before the modulo operation occurs.
*   **`expression2`**: The divisor. Any valid numeric expression. Like the dividend, it is truncated to an integer. It must evaluate to a non-zero value, otherwise a Division by Zero error (`ERR_HOW`) is raised.

## 2. Description & Usage

The `MOD` operator performs arithmetic modulo division. It calculates the remainder of `expression1` divided by `expression2`. 

Before the division occurs, both expressions are converted to integers using truncation (towards zero). This means the fractional part of any floating-point number is simply discarded. 

The resulting remainder takes the sign of the dividend (`expression1`). This behavior conforms to symmetric modulo operations where `(-10) MOD 3` evaluates to `-1` and `10 MOD (-3)` evaluates to `1`. 

**Usage Notes:**
*   If `expression2` evaluates to `0`, the interpreter immediately halts execution of the statement and throws an `ERR_HOW` (Division by Zero) exception.
*   Because the operands are converted to integers, `10.9 MOD 3.2` will be treated as `10 MOD 3`, resulting in `1`.
*   The `MOD` operator evaluates with the same precedence as standard multiplication and division (`*`, `/`), but after exponentiation (`^`).

## 3. Code Examples

**Example 1: Basic Modulo Arithmetic**
```basic
10 A = 10 MOD 3
20 PRINT A
```
*Output:*
```
1
```
*Explanation:* `10` divided by `3` is `3` with a remainder of `1`.

**Example 2: Floating-Point Truncation**
```basic
10 X = 15.7 MOD 4.9
20 PRINT X
```
*Output:*
```
3
```
*Explanation:* `15.7` is truncated to `15`, and `4.9` is truncated to `4`. `15 / 4` is `3` with a remainder of `3`.

**Example 3: Negative Dividends and Divisors**
```basic
10 PRINT (-10) MOD 3
20 PRINT 10 MOD (-3)
30 PRINT (-10) MOD (-3)
```
*Output:*
```
-1
1
-1
```
*Explanation:* The sign of the result always matches the sign of the dividend (the left operand).

**Example 4: Even/Odd Checking**
```basic
10 FOR I = 1 TO 5
20   IF (I MOD 2) = 0 THEN PRINT I; "is Even" ELSE PRINT I; "is Odd"
30 NEXT I
```
*Output:*
```
1 is Odd
2 is Even
3 is Odd
4 is Even
5 is Odd
```

## 4. Internal C-Source Mapping

The `MOD` operator is deeply integrated into both the parsing and evaluation phases of the BASIC++ interpreter:
*   **Lexer/Parser (`source/parser/parser_expr.c`)**:
    *   The `KW_MOD` token maps to the `OP_MOD` AST operator node.
    *   Precedence is defined within `parser_expr.c` (returning 10 for precedence level, equivalent to multiplication and division).
*   **Evaluation Engine (`source/core/value.c`)**:
    *   The actual modulo mathematics and type conversion are executed by the `bval_mod(const BValue *a, const BValue *b, int line_num)` function.
*   **AST Runtime (`source/runtime/ast_interpreter.c`)**:
    *   Resolves `BOP_MOD` (binary operator modulo) by invoking `bval_mod`.

## 5. Implementation Details

When the interpreter encounters the `MOD` operator, the expression parser builds a binary operation AST node (`OP_MOD`). During runtime evaluation, this node triggers a call to `bval_mod`.

**Internal Flow of `bval_mod`:**
1.  **Type Validation:** It verifies both `BValue` arguments are numeric. If string types are improperly supplied without conversion, it returns a zeroed integer, assuming type coercion checks are bypassed or caught prior.
2.  **Integer Coercion:** Both arguments are strictly cast to C `long` integers via `bval_to_int()`, which truncates any floating-point components rather than rounding.
3.  **Zero-Divisor Trap:** The right operand (divisor) is evaluated. If it exactly equals `0`, an `ERR_HOW` exception is explicitly raised via `error_raise(ERR_HOW, line_num)`, and execution is interrupted.
4.  **Modulo Execution:** A standard C modulo operation (`ia % ib`) is performed and wrapped into a numeric `BValue` before being returned to the stack.

**Mutability & Constraints:**
*   Do not alter the integer coercion to round instead of truncate, as truncation guarantees historical standard BASIC compatibility.
*   When expanding expression trees, the `OP_MOD` precedence must rigorously remain tied to multiplication (`*`) and division (`/`) to preserve evaluation determinism.
*   If `bval_mod` breaks during heavy memory loads, confirm that `bval_to_int` is not silently failing on corrupted `BValue` pointers. Ensure the AST nodes correctly populate the left and right operands before dispatching.

## 6. Cross-References / See Also

*   **`INT`**: For explicitly truncating or flooring numeric expressions.
*   **`FIX`**: For discarding fractional parts manually.
*   **`REMAINDER`**: Function equivalent for extracting remainders.
*   **`\` (Integer Division)**: Often used in tandem with `MOD` for quotient and remainder calculations.

## 7. Historical Context

*   **GW-BASIC / QBASIC:** The `MOD` operator behaves identically in BASIC++ as it did in classic Microsoft dialects. The truncation of floating-point values prior to the operation, as well as the behavior of signs matching the dividend, is a strict replication of standard 80s/90s PC BASICs.
*   **ECMA-116:** Full BASIC standards often rely on explicit functions like `MOD(a,b)` or `REMAINDER(a,b)`, but the inline `MOD` operator is preserved in BASIC++ due to its ubiquitous usage across codebases.

## 8. Manual Testing Guide

To verify the correct operational behavior of `MOD`, you can execute the following tests manually in the Windows console:

1.  **Launch the interactive interpreter:**
    Open PowerShell or Command Prompt and run:
    `.\basicpp-console.exe`

2.  **Test Standard Arithmetic:**
    ```basic
    PRINT 10 MOD 3
    ```
    *Expected output:* `1`

3.  **Test Floating-Point Truncation:**
    ```basic
    PRINT 10.9 MOD 3.9
    ```
    *Expected output:* `1` (because `10 MOD 3 = 1`)

4.  **Test Negative Numbers:**
    ```basic
    PRINT (-10) MOD 3
    ```
    *Expected output:* `-1`

5.  **Test Division By Zero Exception:**
    ```basic
    PRINT 5 MOD 0
    ```
    *Expected output:* Program throws a `HOW?` or `Division by zero` error message and halts.

6.  **Exit Interpreter:**
    Type `SYSTEM` or `QUIT` (depending on current dialect settings) or press `Ctrl+C`.


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
