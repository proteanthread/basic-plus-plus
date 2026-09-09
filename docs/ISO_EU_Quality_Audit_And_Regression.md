<!--
Title:        ISO/EU Quality Audit and Regression Verification
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/
Generated:    no
Status:       Active
-->

# ISO/EU Quality Audit and Regression Verification

## 1. Executive Overview & Quality Objectives

BASIC++ v6.5.2 enforces automated static quality gates, architectural invariants, and regression verification across all modular library subsystems in accordance with **ISO/IEC 25010:2023** (Software Product Quality Requirements and Evaluation) and the **EU Cyber Resilience Act (CRA 2024)**.

```
+---------------------------------------------------------------------------------------------------+
|                           BASIC++ Quality Gate & Audit Pipeline                                    |
+-----------------------------------+-----------------------------------+---------------------------+
|      ISO/IEC 25010 Quality Gate   |    Static Engine Rule Audit       |   Anti-Hang Test Battery  |
|      (tools/verify_quality.ps1)   |    (tools/audit_engine_rules.py)  |   (tools/run_all_tests.ps1)|
+-----------------+-----------------+-----------------+-----------------+-------------+-------------+
                  |                                   |                               |
                  v                                   v                               v
+-----------------+-----------------+-----------------+-----------------+-------------+-------------+
| Gate 1: Monolith (< 800 lines)    | Rule 1: No legacy \source or \v5  | 89/89 .BAS Suites Passed  |
| Gate 2: 6-Line Standard Header    | Rule 2: 2-param str_release()     | 0 Subprocess Timeouts     |
| Gate 3: 100% // Comment Standard  | Rule 3: Bounded tok.start cmps    | Wall Clock: ~17s          |
+-----------------+-----------------+-----------------+-----------------+-------------+-------------+
                  |                                   |
                  +-----------------+-----------------+
                                    |
                                    v
            +-----------------------+-----------------------+
            |            10 Freestanding Test Executables   |
            |       (boot, runtime, kernel, hardware,       |
            |        server, script, core/flex, TUI, bppc,  |
            |        single-header C17 embedder API)        |
            |                 100% PASSED                   |
            +-----------------------------------------------+
```

---

## 2. ISO/IEC 25010 Quality Gate Verification

The automated quality verification script ([`tools/verify_quality.ps1`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/tools/verify_quality.ps1) / [`tools/verify_quality.sh`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/tools/verify_quality.sh)) evaluates the codebase against strict maintainability and readability requirements:

1. **Gate 1: Monolith Decomposition (Lines $\le$ 800)**
   - All C17 source and header files under `engine/` are bounded to a maximum of 800 lines of code. Large historical modules are decomposed into decoupled, single-responsibility micro-libraries.
   - Current Status: **100% compliant (0 files exceed 800 lines)**.
2. **Gate 2: 6-Line Canonical Header Block Compliance**
   - Every `.c` and `.h` file begins with the standardized 6-line metadata block (`FILENAME`, `LICENSE`, `VERSION`, `NEEDED BY`, `NEEDS`, prose description, `//`, `// ---- Includes ----`).
   - Current Status: **100% compliant across all source files**.
3. **Gate 3: 100% Double-Slash (`//`) Comment Standard**
   - All comments use pure C99/C17 double-slash (`//`) syntax. Legacy C89 block comments (`/* ... */`) are prohibited to eliminate comment nesting hazards.
   - Current Status: **100% compliant**.

---

## 3. Static Architecture & Engine Safety Audit

The static rule auditor ([`tools/audit_engine_rules.py`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/tools/audit_engine_rules.py)) verifies all active source files against engine safety invariants:

- **Rule 1 (Legacy Directory Exclusion)**: Zero imports or references to legacy `\source` or `\v5` trees.
- **Rule 2 (String Lifecycle Ownership)**: All `str_release()` invocations supply explicit two-parameter context (`vm_get_str(vm), str`), eliminating memory leaks and dangling string pointers.
- **Rule 3 (Bounded Token Comparison)**: Zero unbounded `strcmp`/`strcasecmp` on un-terminated `tok.start` pointers.
- **Rule 4 (Union Safety)**: Zero invalid union member accesses on `TOK_IDENT` tokens.
- **Rule 5 (Virtual Device Routing)**: Zero unrouted `printf`/`putchar` calls in statement parsers.

---

## 4. Full Regression Verification & Anti-Hang Guards

Automated test runners ([`tools/run_all_tests.ps1`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/tools/run_all_tests.ps1)) execute all 89 test suites using bounded execution timeouts:
- Zero subprocess timeouts.
- Zero memory corruption issues or unhandled hardware traps.
- 100% test execution pass rate across all vintage dialects and modern extensions.
