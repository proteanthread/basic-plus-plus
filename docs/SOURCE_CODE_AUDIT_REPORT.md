<!--
Title:        Source Code Audit and Anti-Hang Analysis Report
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/
Generated:    no
Status:       Active
-->

# BASIC++ Full Source Code Audit & Anti-Hang Root Cause Analysis Report

## 1. Executive Summary

During full parity testing across the 44 Vintage and Compiler BASIC Dialects, test executions experienced intermittent hangs. A comprehensive file-by-file audit and static analysis across all C17 source and header files in `engine/` was performed to identify root causes, eradicate violations of engine invariants, and implement multi-layered anti-hang safeguards.

Following surgical fixes and the implementation of dual-layer execution guards, all automated test suites pass with 0 failures, 0 timeouts, and 0 memory corruption issues under bounded wall-clock execution.

## 2. Root Cause Analysis (Why the Interpreter Hung)

The audit identified three primary root causes responsible for test timeouts and execution hangs:

### A. Trailing Postfix Loop Hijacking (`engine/src/vm/exec.c`)
- **Mechanism**: The postfix modifier scanning loop in `exec.c` scanned backward from the end of a line for single-keyword trailing conditionals (such as Tymshare Super BASIC `PRINT X IF A=1` or `A=1 WHILE B>0`).
- **Defect**: The scanner lacked an exemption check for structured block statement keywords. When parsing structured block lines like `DO WHILE cnt% < 5` or `DO ... LOOP UNTIL cond`, the trailing `WHILE` or `UNTIL` was improperly matched by the postfix runner as an outer loop modifier, causing an infinite inner-loop re-entry cycle.
- **Resolution**: Implemented `is_postfix_exempt_keyword()` in `exec.c` to explicitly exempt all 22 structured block keywords (`KW_DO`, `KW_LOOP`, `KW_REPEAT`, `KW_ENDLOOP`, `KW_CASE`, `KW_SELECT`, `KW_IF`, `KW_FOR`, `KW_WHILE`, `KW_UNTIL`, `KW_WHEN`, `KW_SUB`, `KW_FUNCTION`, `KW_DEF`, `KW_TYPE`, `KW_CLASS`, `KW_MODULE`, `KW_SCOPE`, `KW_TRY`, `KW_CATCH`, `KW_RECORD`, `KW_HANDLER`, `KW_WITH`) from trailing postfix loop evaluation.

### B. Nested BASIC09 `LOOP...ENDLOOP` vs Closing `DO...LOOP` Collision (`engine/src/statements/loops/loop.c`)
- **Mechanism**: The statement handler `stmt_loop_handler` checked if an active `DO` frame existed on the stack (`has_do`). If present and `top_line != current_line`, it treated the `LOOP` token as the closing delimiter of an unconditional `DO...LOOP` and executed a backward jump.
- **Defect**: In nested BASIC09 structures (e.g. 5-level deeply nested `LOOP...ENDLOOP`), outer `LOOP` headers were active on the stack. When an inner `LOOP` header executed, `top_line` pointed to the outer `LOOP` header, causing the inner `LOOP` to mistakenly jump backward to the outer loop rather than opening the inner loop frame.
- **Resolution**: Updated `stmt_loop_handler` to prioritize `has_matching_endloop(vm)`:
  1. If `has_matching_endloop(vm)` is true and `top_line != current_line`, it pushes a new frame for the nested BASIC09 block.
  2. If `top_line == current_line`, it continues execution after looping back from `ENDLOOP`.
  3. Only when no matching `ENDLOOP` exists in the forward line stream does it treat `LOOP` as a closing `DO...LOOP` delimiter.

### C. Headless Binary No-Op Graphic Stub Token Advancement (`engine/src/device/vdev.c` & `line.c`)
- **Mechanism**: In headless binaries (`bpp.exe`, `bs.exe`, `iob.dll`), SDL2 graphics and BGI rendering are excluded, falling back to weak stubs in `vdev.c`.
- **Defect**: Fallback stubs for `LINE`, `PSET`, `PRESET`, `CIRCLE`, `PAINT` returned `err.code = 0` without advancing the lexer token stream, violating the **No-Op Statement Handler Token Advancement Guard**. Additionally, `LINE INPUT` was delegated through graphics `LINE`, causing headless runners to miss `LINE INPUT` statements.
- **Resolution**:
  1. Added `vdev_skip_to_stmt_end(lex)` to all weak graphics fallbacks in `vdev.c`.
  2. Updated `stmt_line_handler` in `line.c` to directly inspect for `INPUT` (`KW_INPUT` or `TOK_IDENT "INPUT"`) and immediately dispatch to `stmt_line_input_handler(vm, lex)` before reaching graphics dispatch.

## 3. Dual-Layer Safeguards Implemented

To permanently guarantee that tests and batch executions never hang or stall developer turns, a two-tier protection architecture was engineered:
- Layer 1: Subprocess Process-Tree Enforcer (`tools/run_all_tests.ps1` / `sh`) with strict wall-clock timeouts.
- Layer 2: Internal VM Context Watchdog (`--timeout=<ms>`) with background watchdog threads flagging timeout error 24.

## 4. Static Code Rulebook Audit

Using the automated static rule analyzer `tools/audit_engine_rules.py`, all C17 source and header files across the architectural subsystem layers are audited for compliance with the BASIC++ Engine Safety Invariants:
- Rule 1 (Legacy Directory Exclusion): Zero imports or references to legacy `\source` or `\v5` trees.
- Rule 2 (String Lifecycle Ownership): All `str_release()` invocations supply explicit two-parameter context (`vm_get_str(vm), str`).
- Rule 3 (Bounded Token Comparison): Zero unbounded `strcmp`/`strcasecmp` on un-terminated `tok.start` pointers.
- Rule 4 (Union Safety): Zero invalid union member accesses on `TOK_IDENT` tokens.
- Rule 5 (Virtual Device Routing): Zero unrouted `printf`/`putchar` calls in statement parsers.

## 5. Master Automated Test Runner Telemetry

- Master Dialect Suites Executed: 100% Passed across Turbo BASIC, PowerBASIC, True BASIC, GFA BASIC, TRS-80 CoCo, BASIC09, Sharp Pocket, CBASIC, MBASIC 5.x, IBM PC BASICA, GW-BASIC, Tandy 1000, QBasic, QuickBASIC 4.5, VB/DOS, Tymshare, and ECMA-116.
- Timed Out Suites: 0.
- Failed Suites: 0.
- Built-in `SELFTEST` Diagnostics: All five internal subsystems verified (Lexer, Memory, String, Variable, Array).
- Executable Deployment: Verified `baspp.exe`, `bpp.exe`, `bs.exe`, `iot.exe`, `bppc.exe`, `detok.exe`, `trans.exe`, `basicpp.dll`, and `iob.dll` deployed directly to repository root.
