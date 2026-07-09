# ON...GOSUB

### 1. Syntax & Parameters
`ON <expression> GOSUB <target_1>, [target_2], ..., [target_n]`

*   **`expression`**: A numeric expression that evaluates to an integer index (1-based).
*   **`target_n`**: A comma-separated list of integer line numbers or alphanumeric labels representing the possible subroutine targets.

### 2. Description & Usage
The `ON...GOSUB` command evaluates a numeric expression and branches to the n-th subroutine target in the provided list. Just like a standard `GOSUB`, it pushes the return address (the statement immediately following the `ON...GOSUB`) onto the internal call stack. When the target subroutine completes and executes a `RETURN`, execution resumes after the `ON...GOSUB` statement.

**Usage:**
This statement is ideal for routing execution to specialized handlers (e.g., parsing distinct command types, handling specific user inputs, or processing state-specific updates) without complex conditional branching.

**Edge Cases & Boundary Conditions:**
*   **Index Out of Bounds (Zero or Negative):** If `expression` evaluates to 0 or a negative number, `ON...GOSUB` throws an `ERR_ILLEGAL_FUNCTION_CALL`.
*   **Index Out of Bounds (Too Large):** If `expression` evaluates to a number greater than the number of targets provided in the list, execution silently falls through to the next physical line in the program, and *no* return address is pushed to the stack.
*   **Floating-Point Evaluation:** If `expression` evaluates to a floating-point number, it is rounded to the nearest integer.
*   **Missing Targets:** Empty slots or trailing commas in the list (e.g., `ON X GOSUB 100,,300`) cause an `ERR_SYNTAX`.
*   **Stack Overflow:** Because it acts like `GOSUB`, repetitive branching without a matching `RETURN` will rapidly consume the call stack, resulting in an `ERR_STACK_OVERFLOW`.

### 3. Code Examples

**Basic Routing with Subroutines:**
```basic
10 INPUT "Select action (1-Format, 2-Copy, 3-Delete): ", A
20 ON A GOSUB 100, 200, 300
30 PRINT "Action Complete."
40 END
100 PRINT "Formatting..." : RETURN
200 PRINT "Copying..." : RETURN
300 PRINT "Deleting..." : RETURN
```

**Index Fall-Through:**
```basic
10 X = 5
20 ON X GOSUB 100, 200, 300
30 PRINT "X was out of range. Fall-through executed."
40 END
```

### 4. Internal C-Source Mapping
*   `source/parser/parser_branch.c`
*   `source/virtual/vm_branch.c`
*   `source/virtual/vm_stack.c`

### 5. Implementation Details
The statement is parsed by matching `KW_ON`, parsing an expression, and matching `KW_GOSUB` followed by a list of targets.

**Parser Expectations:**
The parser creates an `AST_ON_GOSUB` node. It stores the evaluated `expression` as a child node, alongside a list of target line numbers or labels.

**Struct Mutations:**
*   **`vm->call_stack`**: If a valid branch occurs, a new frame `vm_frame_t` is pushed onto the stack containing the return address.
*   **`vm->stack_pointer`**: Incremented by 1 if branching occurs.
*   **`vm->pc` (Program Counter):** Mutated to point to the resolved target line or label.

**Execution Paths & Error Handling:**
1. Evaluate `expression` to an integer `idx`.
2. If `idx <= 0` or `idx > 255`, throw `ERR_ILLEGAL_FUNCTION_CALL`.
3. If `idx > target_count`, yield execution and advance `vm->pc` sequentially. Do NOT push to the call stack.
4. If `1 <= idx <= target_count`, extract the `idx`-th target.
5. Push the return address (the next instruction) to `vm->call_stack`. If `vm->stack_pointer >= MAX_CALL_STACK`, throw `ERR_STACK_OVERFLOW`.
6. Set `vm->pc` to the chosen target from `line_map` or `label_map`. If unresolved, throw `ERR_UNDEFINED_LINE`.

**What to do if it breaks:**
If a `RETURN_WITHOUT_GOSUB` error occurs later in the program, verify that the fall-through case did not accidentally push an invalid frame to the stack. The stack must only be mutated if a branch is definitively taken.

### 6. Cross-References / See Also
*   [`ON...GOTO`](ON_GOTO.md)
*   [`GOSUB`](GOSUB.md)
*   [`RETURN`](RETURN.md)

### 7. Historical Context
`ON...GOSUB` brings the computed branching power of `ON...GOTO` to subroutine calls, effectively acting as an array of function pointers. It was historically used in early interactive fiction games and text-based UI software to route player commands to appropriate subroutines.

### 8. Manual Testing Guide
1. Launch `basicpp-console.exe` (Windows) or `baspp-console` (Linux).
2. To test standard routing, type:
   ```
   10 ON 2 GOSUB 100, 200, 300
   20 PRINT "Done"
   30 END
   100 PRINT "A" : RETURN
   200 PRINT "B" : RETURN
   300 PRINT "C" : RETURN
   ```
   Type `RUN`. Output must be `B` followed by `Done`.
3. To test stack preservation on fall-through, type:
   ```
   10 ON 4 GOSUB 100, 200
   20 PRINT "Fall"
   30 END
   100 PRINT "No" : RETURN
   ```
   Type `RUN`. Output must be `Fall`. Ensure no stack overflow or return error occurs.
4. To test error handling, type:
   ```
   10 ON -1 GOSUB 100
   ```
   Type `RUN`. Ensure it throws `ERR_ILLEGAL_FUNCTION_CALL`.


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
