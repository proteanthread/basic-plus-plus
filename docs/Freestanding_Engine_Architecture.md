<!--
Title:        Freestanding Engine Architecture
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/
Generated:    no
Status:       Active
-->

# Freestanding Engine Architecture

## 1. Architectural Overview

The BASIC++ engine (`libengine`) encapsulates the expression evaluator, AST node hierarchy, expression parser, statement dispatchers, built-in function handlers, and runtime evaluation micro-libraries under strict ISO C17 freestanding standards (§4 ¶6).

All engine modules operate with zero reliance on hosted standard C library (`libc`) headers. All memory management, string transformations, numeric parsing, formatting, and mathematical operations route through the freestanding runtime subsystem (`libcore_runtime`) and the Hardware Abstraction Layer (`libhal`).

---

## 2. Core Subsystems & Architectural Boundaries

### 2.1 Freestanding Runtime & HAL Primitives
All modules in `libengine` consume standard freestanding types and runtime operations:
- **Memory Allocations**: Serviced via `hal_get()->mem.alloc` and `hal_get()->mem.free`.
- **String and Memory Primitives**: Implemented in `runtime/string/` (`runtime_memcpy`, `runtime_memset`, `runtime_memmove`, `runtime_memcmp`, `runtime_strlen`, `runtime_strcmp`, `runtime_strncmp`, `runtime_strchr`, `runtime_strncasecmp`).
- **Number Parsing and Formatting**: Implemented in `runtime/conv/` and `runtime/format/` (`runtime_snprintf`, `runtime_parse_double`, `runtime_format_double`).
- **Mathematical Primitives**: Bit-exact IEEE 754 implementations in `libcore_runtime_math` (`runtime_fabs`, `runtime_floor`, `runtime_ceil`, `runtime_round`, `runtime_trunc`, `runtime_fmod`, `runtime_sqrt`, `runtime_cbrt`, `runtime_exp`, `runtime_log`, `runtime_sin`, `runtime_cos`, `runtime_tan`, `runtime_frexp`, `runtime_ldexp`, `runtime_modf`).

### 2.2 Evaluator & AST Pipeline
- **AST Generation**: `ast_create.c`, `ast_parser.c`, `ast_parse_expr.c`, `ast_parse_stmt.c`, and `ast_parse_block.c` construct syntax trees using `ast_alloc_node` and `ast_free_node` backed by the VM memory allocator.
- **AST Evaluator**: `ast_eval_expr.c` and `ast_eval_stmt.c` evaluate abstract syntax trees non-recursively with safe bounds and strict string ownership lifecycle (`str_release`).
- **Expression Evaluation**: `eval.c`, `eval_op.c`, `eval_ident.c`, `eval_array.c`, `eval_new.c`, and `rpn.c` manage operand and operator stacks on the heap with depth guards (limit 64).

### 2.3 Function Micro-Libraries
Function categories in `engine/src/eval/functions/` operate as decoupled micro-libraries:
- **`bits/`**: Bitwise logic, bit shift, bit multiplexing, and binary manipulation.
- **`datetime/`**: Calendar time and serial date computation using `platform_localtime`.
- **`filesystem/`**: File channel status, descriptors, EOF, LOF, LOC, and directory traversal.
- **`math/`**: Trigonometry, logarithms, exponentials, hyperbolic functions, matrix determinants, cross products, and financial functions (PMT, FV, PV, NPER, RATE).
- **`string/`**: Substring manipulation, character conversion, hex/octal formatting, MBF binary representation, Radix-50, and sound token parsing.
- **`system/`**: OS introspection, screen cursor locations, free memory reporting, timer ticks, and environment variable lookups routed via `libplatform`.
- **`ui/`**: TUI dialog widgets and coordinate query functions.

---

## 3. Verification & Test Coverage

1. **Freestanding Test Executables**:
   - `runtime_freestanding_test.exe`: 100% Passed.
   - `kernel_freestanding_test.exe`: 100% Passed.
   - `boot_freestanding_test.exe`: 100% Passed.
2. **Master Dialect Suites**:
   - `vintage_ecosystems_master.bas`: 14/14 Packages Passed.
   - `qb_vbdos_master.bas`: 10/10 Packages Passed.
   - `vintage_deep_fuzz_stress.bas`: 8/8 Tests Passed.
   - `timeshare_master_comprehensive.bas`: 16/16 Passed.
   - `oop_master.bas`: 8/8 Passed.
   - `family_ms_baseline_all.bas`: 8/8 Passed.
   - `gwbasic_qbasic_compat.bas`: 7/7 Passed.
