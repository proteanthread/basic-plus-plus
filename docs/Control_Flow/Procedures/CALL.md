# CALL

### 1. Syntax & Parameters
`CALL <subprogram_name> [(<argument_list>)]`

*   **`subprogram_name`**: The alphanumeric identifier of the subroutine being called.
*   **`argument_list`**: A comma-separated list of arguments (variables, literals, or expressions) passed to the subroutine. These must match the parameter types and count defined in the corresponding `SUB` or `DECLARE` statement.

### 2. Description & Usage
The `CALL` statement transfers control to a subprogram (defined by `SUB`). When the subprogram finishes executing (via `END SUB` or `EXIT SUB`), control returns to the statement immediately following the `CALL`.

**Usage:**
In modern BASIC dialects like QBASIC and BASIC++, procedures (`SUB`) offer local scoping, parameter passing by reference or value, and better modularity compared to classic `GOSUB`. The `CALL` keyword is optional if the subprogram is declared, but it forces arguments to be passed in parentheses.

**Edge Cases & Boundary Conditions:**
*   **Argument Mismatch:** Passing the wrong number of arguments, or arguments of incorrect type (e.g., passing a string when a numeric is expected), results in an `ERR_TYPE_MISMATCH` or `ERR_ARG_COUNT`.
*   **Undefined Subprogram:** Calling a subprogram that has not been defined or declared throws an `ERR_UNDEFINED_PROC`.
*   **Recursion Limits:** Deep recursion (calling a subprogram from within itself repeatedly) will eventually exhaust the call stack, leading to an `ERR_OUT_OF_MEMORY` or `ERR_STACK_OVERFLOW`.
*   **Pass by Reference vs. Value:** By default, variables are passed by reference. Enclosing a variable in extra parentheses, e.g., `CALL MySub((X))`, evaluates it as an expression, effectively passing it by value.

### 3. Code Examples

**Standard Procedure Call:**
```basic
DECLARE SUB PrintGreeting (Name$)

CALL PrintGreeting("Alice")
CALL PrintGreeting("Bob")
END

SUB PrintGreeting (Name$)
  PRINT "Hello, "; Name$
END SUB
```

**Passing by Reference vs Value:**
```basic
DECLARE SUB ModifyValue (X)

A = 10
CALL ModifyValue(A)
PRINT "After ref call, A = "; A  ' Outputs 20

A = 10
CALL ModifyValue((A))
PRINT "After val call, A = "; A  ' Outputs 10
END

SUB ModifyValue (X)
  X = X * 2
END SUB
```

### 4. Internal C-Source Mapping
*   `source/parser/parser_procedures.c`
*   `source/virtual/vm_procedures.c`

### 5. Implementation Details
The `CALL` instruction is identified in the Lexer by the `KW_CALL` token.

**Parser Expectations:**
The parser expects an identifier (`subprogram_name`) followed by an optional, parenthesized list of expressions (`argument_list`). It validates the number and types of arguments against the procedure's signature if it has been previously declared. It generates an `AST_CALL` node.

**Struct Mutations:**
*   **`vm->call_stack`:** A new frame is pushed onto the stack containing the return address (`vm->pc`), local variable scope, and resolved parameter mappings.
*   **`vm->pc`:** Updated to point to the first AST node inside the target `SUB` block.

**Execution Paths & Error Handling:**
1. Evaluate all arguments in the `argument_list`.
2. Locate the target subroutine in the global procedure registry (`vm->proc_registry`).
3. If not found, throw `ERR_UNDEFINED_PROC`.
4. Verify argument count and type compatibility. Throw `ERR_TYPE_MISMATCH` or `ERR_ARG_COUNT` on failure.
5. Push a new stack frame onto `vm->call_stack`.
6. Map evaluated arguments to the subroutine's local parameter variables.
7. Branch `vm->pc` to the target `SUB` body.

**What to do if it breaks:**
If variables are not updating as expected after a `CALL`, check the parameter mapping logic in `vm_procedures.c` to ensure pass-by-reference pointers are correctly resolving to the original variable addresses in the parent scope. Verify that the call stack is not overflowing during deep recursion scenarios.

### 6. Cross-References / See Also
*   [`SUB`](SUB.md)
*   [`DECLARE`](DECLARE.md)
*   [`FUNCTION`](FUNCTION.md)
*   [`GOSUB`](../Branching/GOSUB.md)

### 7. Historical Context
In GW-BASIC, `CALL` was primarily used to invoke machine language routines stored at specific memory addresses, similar to `DEF USR`. With the introduction of QBASIC, `CALL` became the standard way to invoke structured subprograms (`SUB`). BASIC++ supports the modern QBASIC-style `CALL`, focusing on high-level procedural programming with local scope and parameters.

### 8. Manual Testing Guide
1. Launch `basicpp-console.exe` (Windows) or `baspp-console` (Linux).
2. To test standard calling and parameter passing, type:
   ```
   10 DECLARE SUB TestSub(X)
   20 CALL TestSub(5)
   30 END
   40 SUB TestSub(X)
   50 PRINT "Value:"; X
   60 END SUB
   ```
   Type `RUN`. Output must be `Value: 5`.
3. To test pass-by-reference, type:
   ```
   10 DECLARE SUB DoubleIt(V)
   20 A = 10
   30 CALL DoubleIt(A)
   40 PRINT A
   50 END
   60 SUB DoubleIt(V)
   70 V = V * 2
   80 END SUB
   ```
   Type `RUN`. Output must be `20`.
4. To test undefined procedure error, type:
   ```
   10 CALL MissingSub(1)
   ```
   Type `RUN`. Ensure it throws `ERR_UNDEFINED_PROC`.


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
