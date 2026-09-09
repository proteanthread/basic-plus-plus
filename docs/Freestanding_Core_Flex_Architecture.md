<!--
Title:        Freestanding Core and Flex Metaprogramming Architecture
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/
Generated:    no
Status:       Active
-->

# Freestanding Core and Flex Metaprogramming Architecture

## 1. Architectural Overview

The BASIC++ core REPL, system introspection, dynamic metaprogramming, and flex subsystems (`libcore` and `libflex`) implement dialect reflection, symbol aliasing, lexical scoping, keyword overrides, documentation generation, and built-in diagnostic testing under strict ISO C17 freestanding compliance (§4 ¶6).

All direct dependencies on hosted standard C file streams (`FILE*`), hosted libc headers (`<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<ctype.h>`), heap allocators, and string formatting routines have been migrated to the pure C17 freestanding Runtime Library (`runtime_*`) and the Hardware Abstraction Layer (`IoHandle`, `hal->io.*`, `hal->mem.*`).

---

## 2. Converted Source Inventory & Subsystems

### 2.1 Core REPL & Introspection Subsystem (`libcore`)
- **Documentation Generator ([`engine/src/docgen/docgen.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/docgen/docgen.c))**: Generates JSON schema exports, Markdown reference manuals, and HTML portals using `hal->io.file_open`, `hal->io.file_write`, `hal->io.file_close`, and `runtime_vsnprintf`.
- **Category Reflection ([`engine/src/statements/dialect/category.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/statements/dialect/category.c))**: Implements the `CATEGORY` statement using freestanding string operations.
- **Help and Catalog Dispatch ([`engine/src/statements/dialect/help.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/statements/dialect/help.c))**: Implements interactive `HELP` and `CATALOG` lookups using HAL file streams.
- **Host Introspection ([`engine/src/statements/dialect/introspection.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/statements/dialect/introspection.c))**: Provides `HOSTNAME` and `USERNAME` statements with zero hosted libc dependencies.
- **Built-in Self-Testing ([`engine/src/statements/dialect/selftest.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/statements/dialect/selftest.c))**: Drives the `SELFTEST` diagnostic suite for kernel, VM, and memory validation.
- **Category Query Function ([`engine/src/eval/functions/system/category.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/eval/functions/system/category.c))**: Implements `CATEGORY$` using freestanding string primitives.

### 2.2 Dynamic Metaprogramming & Flex Subsystem (`libflex` & Runtime)
- **Symbol Aliasing ([`engine/src/statements/dialect/alias.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/statements/dialect/alias.c))**: Implements `ALIAS`, `ALIAS OPER`, `ALIAS LIST`, `ALIAS SAVE`, and `ALIAS LOAD`.
- **Dynamic Keyword Registry ([`engine/src/statements/dialect/keyword.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/statements/dialect/keyword.c))**: Implements `KEYWORD SET/GET/LIST/CLEAR` for dynamic vocabulary reconfiguration.
- **Statement Overriding ([`engine/src/statements/dialect/override.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/statements/dialect/override.c))**: Implements `OVERRIDE WITH GOSUB/SUB` statement redirection.
- **Pattern Cleansing ([`engine/src/statements/dialect/remove.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/statements/dialect/remove.c))**: Implements `REMOVE` and `REMOVE$` in-place scalar and array pattern cleansing.
- **Lexical Scoping ([`engine/src/statements/dialect/scope.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/statements/dialect/scope.c))**: Implements `SCOPE DISABLE/ENABLE/HOOK/PRIVATE/MODULE/BEGIN/END/PROTECT`.
- **Scope State Machine ([`engine/src/scope/scope.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/scope/scope.c))**: Manages the lexical scope stack and execution hooks.
- **Keyword Properties ([`engine/src/runtime/keyword_props.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/runtime/keyword_props.c))**: Maintains keyword attribute bitmasks and property descriptors.
- **Statement Override Table ([`engine/src/runtime/override.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/runtime/override.c))**: Dispatches overridden statement vectors.

---

## 3. Verification & Test Suite

1. **Unit Test Suite ([`tests/core_flex_freestanding_test.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/tests/core_flex_freestanding_test.c))**:
   - Documentation generator subsystem (JSON export, Markdown manual, HTML portal).
   - ALIAS and KEYWORD properties subsystems (registration, lookup, removal, operator aliasing, property tables).
   - SCOPE and OVERRIDE subsystems (keyword disabling/enabling, execution hooks, private/protected symbols, block depths, namespaces, statement overriding).
   - CATEGORY, CATEGORY$, and Introspection (statement, function, hostname, username).
   - Built-In SELFTEST diagnostics (five internal subsystems verified).
2. **Freestanding Test Execution**: All seven freestanding suites pass with 100% verification.
3. **Master Regression Coverage**:
   - `tests/qb_vbdos_master.bas`: 10/10 Packages PASSED (100%).
   - `tests/vintage_ecosystems_master.bas`: 14/14 Packages PASSED (100%).
   - `tests/vintage_deep_fuzz_stress.bas`: 8/8 Tests PASSED (100%).
   - Batch runner execution: `bs -c "PRINT 1+1"` verified with output `2`.
