BASIC++ (IB) Interpreter
Core Implementation

I don't care what you do with my code, just don't take my code and sell and don't take my code, modify my code, and sell it. This code is not for sale.

The subject of this project is a minimal, portable interpreter for the BASIC language. The implementation is contained within a single C source file, a design choice intended to facilitate maximum portability and ease of distribution across disparate operating systems and architectures with minimal build-system dependencies. This implementation is architected as a foundational framework, explicitly designed not as a monolithic, static entity, but as a core upon which future capabilities are intended to be constructed via a formalized modular architecture.

Primary design considerations are memory footprint optimization and source code lucidity. The former objective ensures viability for resource-constrained environments, such as 8-bit microcontrollers, embedded systems, or legacy hardware emulators where memory is a severely limited commodity. The latter objective, lucidity, renders the source code suitable for pedagogical review. It serves as a clear, annotated example of foundational interpreter design, illustrating concepts such as tokenization, recursive-descent parsing, and environment management in a comprehensible manner. Execution velocity, while a consideration, is posited as a subordinate objective to these primary goals. This trade-off is deliberate; clarity and portability have been prioritized over complex, platform-specific optimizations, ensuring that the core remains verifiable and maintainable. The architecture is, therefore, explicitly provisioned for future, modular extensibility as outlined in Section 6.


# Section 1: Core Features
The interpreter provides an implementation of a rudimentary BASIC dialect, designated "IB Core." This dialect is predicated entirely on 8-bit signed integer arithmetic, a fundamental design constraint of this core implementation. This constraint dramatically simplifies the runtime engine, obviates the need for a floating-point unit or complex software-based floating-point libraries, and establishes a deterministic mathematical environment.

## 1.1. Data Type
All numeric operations, constants, and variable storage are constrained to 8-bit signed integers, commensurate with the signed char data type in the C language. This provides a supported numerical range from -128 to +127. Any arithmetic operation resulting in an overflow or underflow of this range (e.g., 127 + 1) will exhibit 'wrap-around' behavior, as is characteristic of two's-complement arithmetic. This behavior is deterministic: an operation exceeding +127 will wrap to -128, and one below -128 will wrap to +127. Furthermore, all division operations (/) are integer-based, meaning any fractional component of a quotient is truncated, not rounded. This specific behavior (e.g., 7 / 3 evaluates to 2) is essential to the definition of an "integer-only" dialect.

## 1.2. Variable Storage
A static allocation of 26 numeric variables is provided, identified by the alphabetical characters A through Z. This fixed, minimal namespace is a deliberate design choice that simplifies the interpreter's variable management (a direct array-index lookup) and ensures a predictable, static memory footprint. No mechanisms for dynamic variable creation, variable-length names, aliasing, or user-defined data types are provided within this core implementation. All variables are global in scope and are initialized to zero upon the execution of the RUN directive.

## 1.3. Parser
Expression evaluation is conducted via a simple, recursive-descent parser. A defining characteristic of this parser is its strict left-to-right evaluation, which does not observe standard mathematical operator precedence. For example, the expression 3 + 4 * 5 will be evaluated as (3 + 4) * 5, yielding 35. This contrasts with a standard precedence-observing parser, which would evaluate 3 + (4 * 5) to yield 23. This simplification is conducive to a minimal parser implementation and is a documented characteristic of this dialect. Support for the four fundamental arithmetic operators (+, -, *, /) is included. Sub-expressions encapsulated in parentheses are syntactically supported and are evaluated recursively. This permits the explicit enforcement of evaluation order by the programmer (e.g., 3 + (4 * 5) will be correctly evaluated as 23), providing a necessary manual override for the parser's non-precedence behavior.

## 1.4. Implemented Directives (Commands)
The set of implemented language commands provides foundational capabilities for program flow, data manipulation, and termination. These directives include:
Data I/O: PRINT, LPRINT, LET (for variable assignment), and INPUT (for user data entry from the console).
Program Flow: GOTO, GOSUB, RETURN (for unconditional branching and subroutine logic), and IF...THEN (for single-line conditional execution).
Program Structure & Environment: REM (for program annotation), END, STOP (for program termination), BEEP (for audio signaling), SYSTEM (reserved for future module use), QUIT, and EXIT (for terminating the interpreter process).

## 1.5. Environment Directives
A distinct set of directives, which are not intended for use within a stored program line (i.e., they cannot be preceded by a line number), are provided for managing the runtime environment and the program itself. These directives operate at the "edit" level. They include: RUN (to initiate execution), LIST (to display the stored program), NEW (to clear program memory), SAVE (to persist program memory to storage), and LOAD (to retrieve a program from storage).

## 1.6. Input/Output Operations
The core implementation provides two distinct output directives. The PRINT directive supports the output of both string literals (delimited by quotation marks) and the current value of any of the 26 numeric variables to the primary console display (standard output). The LPRINT directive, while syntactically similar, is specified to redirect its output to an external file designated as lprint.out. This mechanism simulates the behavior of a physical line printer device, providing a method for persistent data logging. This file-based implementation serves as the portable foundation for the project's long-term goal of supporting PDF or PostScript output via a more advanced plugin.


# Section 2: Compilation (GCC)
The C source code is designed for high portability and is compilable on systems featuring a standards-compliant C compiler. The following examples utilize the GNU Compiler Collection (GCC), and the provided flags are recommended for specific build goals.

## 2.1. Compilation for Portability and Size *(Recommended)*

gcc -Wall -Os -o ib ib.c

This incantation invokes the compiler with -Wall to enable all high-priority warnings, a best practice for identifying potential portability issues or unsafe code. Crucially, it uses -Os, which instructs the compiler to optimize specifically for the size of the resulting executable binary. This optimization level is often the primary concern in memory-constrained systems, such as the target CP/M or embedded environments.

## 2.2. Compilation for Execution Speed

gcc -Wall -O2 -o ib ib.c

This command, by contrast, uses the -O2 flag. This flag enables a more aggressive set of optimization passes (such as loop unrolling and function inlining) focused on increasing the execution velocity of the program. This may come at the cost of a slightly larger binary file and a significantly longer compilation time. This build is suitable for desktop systems where performance is prioritized over footprint.

## 2.3. Compilation for Debugging</i> *(Symbolic Inclusion)*

gcc -Wall -g -o ib ib.c

This command utilizes the -g flag to instruct the compiler to include debugging symbols (such as DWARF) within the final executable. This symbolic information is essential for using a debugger (such as GDB) to trace program execution, inspect variables, and analyze the call stack, which is an indispensable part of the development and troubleshooting process.


# Section 3: Memory Allocation and Layout
The user-addressable memory within the interpreter, as well as its internal state management structures (such as the GOSUB stack), are defined by static, fixed-size arrays. The dimensions of these arrays are established at compile-time via #define constants, ensuring a predictable and static memory footprint for the entire interpreter process.

## 3.1. Memory Allocation Table
The default allocation, which directly dictates the interpreter's capacity, is as follows:

| Memory Area       | Constant      | Size         | Calculation (assuming 4-byte int)     | Total Size   |
| :---------------- | :------------ | :----------- | :------------------------------------ | :----------- |
| Program Storage   | MAX_LINES     | 500 lines    | 500 * (127 chars + 4 bytes for line#) | 64.0 KB      |
| Variable Storage  | NUM_VARIABLES | 26 vars      | 26 * 1 byte (signed char)             | 26 bytes     |
| GOSUB Stack       | STACK_SIZE    | 64 levels    | 64 * 4 bytes (int)                    | 256 bytes    |
| **Total**         |               |              |                                       | **~64.2 KB** |

It is noted that the "kbytes Free" message, displayed at interpreter initialization, reports exclusively on the 'Program Storage' allocation (the Line structure array), which, following integer division, equates to 63 KB. This figure does not include the negligible-by-comparison variable and stack allocations, as it is intended to inform the user of the space available for their BASIC program lines.

## 3.2. Adjustment of Memory Allocations
Alterations to these memory limitations are effectuated by modifying the appropriate #define pre-processor constants within the ib.c source file. Subsequent recompilation of the interpreter is mandatory for such changes to take effect. This compile-time configuration is a deliberate design choice, precluding runtime memory negotiation (e.g., malloc()). This approach ensures that the interpreter's resource requirements are fixed and verifiable, a critical attribute for high-reliability systems, embedded applications, or legacy operating systems (like FreeDOS) where dynamic memory management is complex or unreliable.


# Section 4: Operational Use
The interpreter operates via a standard REPL (Read-Evaluate-Print Loop) interface, a common paradigm for interactive language shells. This interface provides two distinct contexts for operation: Direct Mode and Program Mode.

## 4.1. Direct Mode
The direct, or "immediate," execution context is invoked when directives are entered without a preceding line number (e.g., PRINT 10 + 5). Such directives are evaluated and executed immediately upon entry. This mode is principally utilized for testing, for debugging individual commands, for performing simple "calculator" style calculations, or for inspecting the current state of variables (e.g., PRINT A).

## 4.2. Program Mode
The "stored program" context is invoked when directives are entered with a preceding line number (e.g., 10 PRINT "HELLO"). Such lines are not executed; instead, they are parsed and passed to the store_line() function, which inserts them into the 'Program Storage' array. This array is maintained in a state sorted by line number. This allows for the construction of a persistent (session-local), ordered program that can be executed as a whole.

## 4.3. Program Execution
The RUN directive initiates sequential execution of the stored program. This directive is a destructive operation in that it first clears the 'Variable Storage' and 'GOSUB Stack' to a zeroed state, ensuring that the program executes in a clean, predictable environment (i.e., all variables are 0, and the stack is empty). Execution then begins at the lowest extant line number found in the 'Program Storage'. The LIST directive provides a textual representation of the in-memory program, displaying all currently stored lines in ascending numerical order to the console.


# Section 5: Halting Non-Terminating Execution
In the event a BASIC program enters a non-terminating (i.e., endless) loop, which is a common possibility given the GOTO directive, execution of the interpreter process may be forcibly terminated. This is accomplished by issuing an interrupt signal (SIGINT) via the Ctrl+C key combination from the controlling terminal. This action is not handled by the interpreter itself; rather, it is handled by the host operating system (e.t., the Linux kernel or the FreeDOS command shell), which will unconditionally halt the interpreter process and return control to the host command-line shell.


# Section 6: Future Expansion Trajectory
The architecture of this interpreter is explicitly provisioned for future expansion. The current monolithic design is a foundational step, intended to be refactored into a dynamic-dispatch engine as specified in the project's internal architecture proposals. The following classifications for extensibility have been identified. These systems are not yet implemented but constitute the formal specification and roadmap for future development.

## 6.1. Addons
This classification is designated for the most advanced extensibility, involving the incorporation of inline foreign language code. This system would provide meta-directives (e.g., $LANG: C) to allow a user to embed, compile, and link source code from other languages, such as Assembly, Pascal, or C, directly within a BASIC program file. This represents the ultimate goal of a mixed-language development environment, likely implemented via a "BASIC-to-C" transpiler and external compiler-chaining.

## 6.2. Merge
This classification refers to functionality for BASIC source code amalgamation, specifically the $MERGE directive. This system must maintain strict compliance with the behavioral standards of ECMA-55 (Minimal BASIC), ECMA-116 (Full BASIC), and/or QBASIC/QuickBASIC. Its function is to load a BASIC program file from storage and combine it with the program already resident in memory, with lines from the incoming file overwriting any pre-existing lines with identical numbers or subroutines. This is the foundational pillar for user-level code sharing.

## 6.3. Modules
This classification defines the primary system for C-level code extensibility. A "Module" is a compiled C-code entity (e.g., an object file or shared library) that adds new keywords and syntactic features to the interpreter. This system is responsible for language syntax modification, enabling the creation of dialect-specific feature sets (e.g., adding a GRAPHICS module to provide PSET and LINE, or a SOUND module to provide PLAY). This is the mechanism by which the interpreter will evolve from "Core" to "Full" BASIC.

## 6.4. Plugins
This classification defines a specialized subset of Modules. A "Plugin" is a C-code module designated for low-level hardware mapping, system emulation, and direct memory interfacing. A Plugin functions as a "driver," abstracting the hardware. For example, a SOUND Module (Pillar 6.3) would provide the SOUND keyword, but it would call a SOUND Plugin (e.g., pc_speaker.plugin for DOS or oss.plugin for Linux) to actually generate the audio. This architectural separation of semantics (Module) from implementation (Plugin) is the key to achieving cross-platform portability for hardware-dependent features.
