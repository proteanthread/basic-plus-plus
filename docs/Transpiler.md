<!--
Title:        Transpiler
Tier:         2
Applies to:   BASIC++ v6.5.2 (trans, trans.exe)
Authority:    engine/src/compiler/trans_c17.c, engine/include/compiler/
Generated:    no, hand-written
Status:       current
-->

# BASIC++ Transpiler Architecture & C17 Code Generation Reference

The authoritative specification for the BASIC++ source-to-source transpiler (`trans` / `trans.exe`), focusing on the active ISO C17 code generation target.

---

## 1. What It Does

1. Translates BASIC++ source code into clean, human-readable, freestanding-compliant ISO C17 source code.
2. Emits structured C17 control flow constructs (`for`, `while`, `do-while`, `switch-case`, `if-else`) matching BASIC control structures.
3. Preserves original BASIC source lines as synchronized C code comments (`/* Line 10: ... */`).
4. Maps BASIC scalar variables and array allocations to typed C17 identifiers (`bpp_<name>`).
5. Generates calls to the lightweight BASIC++ C runtime library for string operations, formatted I/O, and mathematical routines.
6. Emulates vintage `GOSUB` / `RETURN` stacks using C functions and label tables.
7. Emulates `ON ERROR GOTO` and structured `TRY...CATCH` exception handling via `setjmp` / `longjmp` scaffolds.
8. Supports single-file compilation via `--inline-runtime`, embedding the required C runtime functions directly into the generated C source.
9. Performs static optimization passes including constant folding, dead-code elimination, and scalar type specialization.
10. Operates as an independent build target (`trans.exe` on Windows, `trans` on Linux) deployed directly to the repository root.

---

## 2. Why It Exists

1. Enables BASIC++ programs to compile into native bare-metal machine code with zero interpreter overhead.
2. Produces standard portable C17 code that compiles under GCC, Clang, MSVC, and DJGPP across all major operating systems.
3. Serves as the compilation bridge for resource-constrained embedded microcontrollers and bare-metal systems targets.
4. Allows integration of vintage BASIC algorithms directly into modern C/C++ software pipelines and build trees.
5. Verifies AST semantic correctness and language descriptor parity across interpreted and compiled execution models.
6. Preserves branchless vintage arithmetic invariants (relational truth value `-1` mapped to boolean logic).
7. Conforms to ISO/IEC 25010 Portability, Efficiency, and Maintainability standards.
8. Enforces strict Documentation and Help Mirror Parity between Markdown and plaintext formats.

---

## 3. Invocation and Command-Line Usage

```bash
trans [options] input_file.bas [-o output_file.c]
```

- **`-o <path>`**: Designates target C17 output filename.
- **`--inline-runtime`**: Inlines runtime library functions directly into the emitted C file.
- **`--freestanding`**: Generates freestanding ISO C17 conforming strictly to §4 ¶6 (zero hosted libc headers).
- **`--trace`**: Emits AST transformation tracing to standard output.
