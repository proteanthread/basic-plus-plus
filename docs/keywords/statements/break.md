<!--
Title:        break
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/ast/eval_ast.c
Generated:    no, hand-written
Status:       current
-->

# `BREAK` Keyword Reference

## Source Header

```c
// FILENAME: eval_ast.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for break.
```

## 1. Description & Usage

Immediately exits the innermost active loop (FOR, WHILE, DO), or invokes the interactive debugger breakpoint.

## 2. Syntax

```basic
BREAK
```

## 3. Code Example

```basic
10 REM BREAK Demonstration
20 PRINT "BREAK executed successfully."
```

## 4. Error Conditions

Error 5: Illegal Function Call (outside loop)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Control Flow
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | BREAK |
| Category | Control Flow |
| Syntax | BREAK |
| Description | Immediately exits the innermost active loop (FOR, WHILE, DO), or invokes the interactive debugger breakpoint. |
| Error Summary | Error 5: Illegal Function Call (outside loop) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/eval/ast/eval_ast.c |
