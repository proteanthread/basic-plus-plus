# OPTION VIDEO

## 1. Syntax & Parameters

**Syntax:**
`OPTION VIDEO <mode>`

**Parameters:**
- `<mode>`: A literal named variable representing the desired video standard. Supported values are:
  - `PAL`: Configures the timing multiplier for 50 Hz video systems.
  - `SECAM`: Functionally identical to PAL, configuring the timing multiplier for 50 Hz video systems.
  - `NTSC`: Configures the timing multiplier for 60 Hz video systems.

## 2. Description & Usage

The `OPTION VIDEO` statement allows developers to configure the global timing metric environment for the interpreter, specifically adjusting the internal system's definition of "Jiffies" to match historical 50 Hz (PAL/SECAM) or 60 Hz (NTSC) television and monitor refresh rates. 

In classic BASIC programming, timing loops and software delays were often coupled with screen refreshes (VBLANK intervals). A "Jiffy" originally described a single tick of the system clock, tied to these refresh rates (1/50th of a second for PAL, 1/60th of a second for NTSC). `OPTION VIDEO` sets the `rt->jiffies_multiplier` property in the runtime state, which dictates how the `JIFFIES` function calculates the number of ticks elapsed since system startup.

By default, without being set, it may rely on whatever default initialization exists in the runtime state. Invoking `OPTION VIDEO PAL` scales the elapsed system time (in seconds) by 50, whereas `OPTION VIDEO NTSC` scales it by 60.

This statement is useful when running legacy timing loops, game delays, or simulations where the original logic expected a specific Jiffy progression rate. It allows seamless porting of classic BASIC programs without rewriting all of the timing code.

## 3. Code Examples

**Example 1: Setting up an NTSC timing loop**
```basic
10 OPTION VIDEO NTSC
20 PRINT "Waiting for 60 jiffies (1 second in NTSC)..."
30 START_TIME = JIFFIES
40 IF JIFFIES < START_TIME + 60 THEN GOTO 40
50 PRINT "Done."
```

**Example 2: Cross-compatibility using PAL**
```basic
100 REM Simulate a 50Hz environment for European software
110 OPTION VIDEO PAL
120 START_T = JIFFIES
130 GOSUB 1000 : REM Call some processing routine
140 END_T = JIFFIES
150 PRINT "Processing took "; END_T - START_T; " PAL jiffies."
160 END
1000 REM Dummy subroutine
1010 FOR I = 1 TO 1000 : NEXT I
1020 RETURN
```

## 4. Internal C-Source Mapping

- **Parser Configuration (`source/config/parser_config.c`)**: The `pi_parse_option` function handles the lexical interpretation of `OPTION VIDEO`. It checks the following token (the mode) as a `TOK_NAMED_VAR` and parses "PAL", "SECAM", or "NTSC", directly writing to `rt->jiffies_multiplier`.
- **Runtime State Definition (`source/runtime.h`)**: `RuntimeState` struct contains the `double jiffies_multiplier` property, which persists this configuration throughout the session.
- **Expression Parser (`source/parser/parser_expr.c`)**: The `JIFFIES` function implementation around line 1324 fetches `rt->jiffies_multiplier` to multiply against `vdev_get_time()`.

## 5. Implementation Details

- **Parser Expectations**: The statement demands exact syntax parsing. The lexer will identify `OPTION` as a statement keyword, then `pi_parse_option` reads the next tokens. Both `VIDEO` and the mode (`PAL`, `SECAM`, or `NTSC`) are parsed as `TOK_NAMED_VAR` due to extended variable handling, matched by character string checks.
- **Mutability Constraints**: `rt->jiffies_multiplier` is fully mutable at any point during runtime. Subsequent calls to `OPTION VIDEO` will override the previous settings dynamically, affecting all future evaluations of the `JIFFIES` function.
- **Failure Modes & Troubleshooting**: 
  - If a mode other than PAL, SECAM, or NTSC is provided, the parser triggers `error_raise(ERR_WHAT, line_num)`. Check for misspellings (e.g., `OPTION VIDEO NSTC`).
  - The `JIFFIES` function can temporarily override this global multiplier if passed an argument string (e.g., `JIFFIES("PAL")`), which acts independently of `OPTION VIDEO`.
- **NLP Indexing Note**: The mutation occurs as a direct structural write to a double-precision floating-point field in `RuntimeState`. No memory allocation is involved, ensuring deterministic $O(1)$ execution time.

## 6. Cross-References / See Also

- `JIFFIES`
- `TICKS`
- `OPTION ARITHMETIC`
- `OPTION BASE`
- `OPTION STRICT`

## 7. Historical Context

In legacy systems like the Commodore 64 or ZX Spectrum, the system variable for time was physically tied to hardware interrupts triggered by the video chip drawing the screen. GW-BASIC and IBM PCs relied on the Intel 8253 timer chip (usually ticking at 18.2 Hz), but many graphics-heavy programs were developed assuming either a 50Hz or 60Hz timing metric. `OPTION VIDEO` is a modern BASIC++ extension designed to bridge this compatibility gap, allowing developers to explicitly define the simulated hardware environment timing without altering the underlying OS clock dependencies. 

## 8. Manual Testing Guide

To manually verify the functionality of `OPTION VIDEO` and its effect on `JIFFIES`, follow these steps:

1. Open your terminal in the project directory.
2. Launch the BASIC++ console application:
   - **Windows**: `basicpp-console.exe` (or `basicpp.exe` for SDL)
   - **Linux**: `./baspp-console` (or `./baspp` for SDL)
3. Enter direct mode or write a short program to test NTSC timing:
   ```basic
   OPTION VIDEO NTSC
   PRINT JIFFIES
   ```
   *Expectation*: The value returned should grow by roughly 60 units per second of real time.
4. Now switch to PAL timing:
   ```basic
   OPTION VIDEO PAL
   PRINT JIFFIES
   ```
   *Expectation*: The value returned should grow by roughly 50 units per second.
5. To test failure modes, try an invalid mode:
   ```basic
   OPTION VIDEO CGA
   ```
   *Expectation*: The interpreter should throw a `WHAT?` error indicating a syntax failure.


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
