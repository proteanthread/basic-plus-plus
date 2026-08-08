# SELFTEST

### 1. Syntax & Parameters
`SELFTEST [component_name$]`

Parameters:
- `component_name$` (Optional): A string expression specifying a particular subsystem to test (e.g., "LEXER", "MATH", "STRINGS", "MEMORY"). If omitted, the interpreter runs the complete, exhaustive diagnostic suite across all subsystems. Note the strict string keyword suffix constraint is not enforced on the argument itself, but the argument must evaluate to a string type.

### 2. Description & Usage
The `SELFTEST` command is a powerful internal diagnostic utility built directly into the BASIC++ interpreter. When invoked, it pauses standard script execution and runs a series of hardcoded, highly rigorous C-level unit tests against the interpreter's core subsystems. It verifies the integrity of the Lexer, Value system, String pool, Function registry, Memory allocator, configuration, Parser precedence, Control flow mechanisms, Virtual File System (VFS), and SDL graphics contexts.

This command acts as a built-in regression suite to guarantee that the compiled binary is functioning correctly on the host operating system before running complex BASIC scripts.

**Usage:**
- Use directly from the command line interface: `basicpp-console.exe -c "SELFTEST"`
- Execute within immediate mode (REPL) to verify the environment health.
- Include at the very beginning of automated deployment scripts to ensure interpreter integrity before executing critical code.

**Edge Cases & Boundary Conditions:**
- **Unknown Component Name:** If an unrecognized string is passed (e.g., `SELFTEST "INVALID"`), the interpreter should gracefully reject it and print an error message indicating the component is unknown, without crashing.
- **Memory Pressure:** Running `SELFTEST` involves allocating and deallocating thousands of temporary objects. On highly constrained embedded systems, the exhaustive test might trigger out-of-memory errors. The tests must be designed to clean up after themselves impeccably.
- **Side-Effects on User State:** A well-implemented `SELFTEST` must NOT mutate user variables, clear the screen unexpectedly (unless testing SDL bounds), or alter the loaded program in memory. It must run in a perfectly isolated virtual sandbox or restore state upon completion.

**Side Effects:**
- Generates substantial console output detailing the pass/fail status of various internal C structs and functions.
- Momentarily halts the virtual machine while native C tests execute.
- Heavy utilization of the memory allocator during the test cycle.

**Behavior Constraints:**
- `SELFTEST` is a strictly synchronous, blocking operation.
- The command cannot be used within mathematical expressions or assignments.
- Output from `SELFTEST` cannot be suppressed easily without redirecting standard output at the OS level.

### 3. Code Examples

**Example 1: Full Exhaustive Test**
```basic
10 REM Run all diagnostics
20 SELFTEST
30 PRINT "Interpreter is healthy!"
```
*Expected Output:*
```
--- BASIC++ INTERNAL SELFTEST ---
[PASS] Lexer Initialization
[PASS] Symbol Table Hashing
[PASS] Memory Pool Allocation
[PASS] AST Precedence Rules
...
ALL TESTS PASSED.
Interpreter is healthy!
```

**Example 2: Testing a Specific Subsystem**
```basic
10 REM Only test the string pool
20 SELFTEST "STRINGS"
```
*Expected Output:*
```
--- BASIC++ COMPONENT TEST: STRINGS ---
[PASS] String Allocation
[PASS] String Concatenation
[PASS] Garbage Collection
```

### 4. Internal C-Source Mapping
- `source/virtual/selftest_vm.c`
- `source/parser/selftest_parser.c`
- `source/tests/internal_diagnostics.c` (C-level test definitions)

### 5. Implementation Details
The `SELFTEST` command is parsed as a standalone statement `KW_SELFTEST`. 

**Struct Mutations:**
- During execution, `SELFTEST` does not modify the standard user environment (`sym_table` or user memory). Instead, it spins up temporary isolated environments (`BASIC_VM_STATE` instances) completely decoupled from the main runtime to execute C-level assertions against internal API endpoints.
- It iterates over an array of function pointers defined in the diagnostic suite.
- If a test fails, it mutates the test runner's internal failure counter and prints a red `[FAIL]` block to the virtual console stream.

**Execution Paths:**
- Lexer: Reads "SELFTEST", returns `KW_SELFTEST`.
- Parser: Parses `NODE_SELFTEST`. Evaluates optional string argument.
- Evaluator: Calls `execute_internal_selftests(component_string)`.
- The C backend intercepts the call, suspends the AST evaluator, runs the native C test loop, and then resumes the evaluator.

**Troubleshooting & Mutability:**
- **What can be changed:** Developers can add new functional test endpoints to `internal_diagnostics.c` as new features are added. The output formatting can be updated to be more verbose or strictly machine-readable (e.g., JSON output).
- **What cannot be changed:** The requirement that `SELFTEST` must be isolated. It must absolutely never leak memory or overwrite user variables.
- **If it breaks:** If `SELFTEST` crashes the interpreter, it indicates a severe regression in memory management or pointer safety on 64-bit platforms. Use a native debugger (like GDB or Visual Studio) to catch the segmentation fault within the C test runner. If `SELFTEST` reports false positives, verify that the internal mock objects used by the tests are being properly initialized.

### 6. Cross-References / See Also
- `ASSERT`
- Command-line flags `--log`, `--out`, `--debug`

### 7. Historical Context
`SELFTEST` is a modern addition to BASIC++ designed to align with strict CI/CD and regression prevention mandates. It does not exist in GW-BASIC or QBASIC. By embedding the unit tests directly into the binary, the interpreter acts as its own test runner, guaranteeing identical test coverage across all target platforms (Windows Console, Windows SDL, Linux Console, Linux SDL).

### 8. Manual Testing Guide
To manually verify `SELFTEST` functionality without the automated Python suite:
1. Open a terminal or command prompt.
2. Launch the console executable with debugging enabled: `basicpp-console.exe --log`
3. At the prompt, type `SELFTEST` and press Enter.
4. Verify that the console outputs the internal diagnostic suite results and that all tests show as `[PASS]`.
5. Next, type `SELFTEST "LEXER"` and verify that only the Lexer subsystem tests are executed and passed.
6. Verify that typing `SELFTEST "INVALID"` gracefully reports that the component is unknown without crashing the interpreter.


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
