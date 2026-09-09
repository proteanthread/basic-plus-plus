<!--
Title:        JIT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/program/stmt_jit.c
Generated:    no, hand-written
Status:       current
-->

# `JIT` Keyword Reference

## Source Header

```c
// FILENAME: stmt_jit.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore, libengine, libkernel
// Implements the JIT statement for dynamic runtime JIT/AOT mode configuration
//
// ---- Includes ----
```

## 1. Description & Usage

Configures the Just-In-Time and Ahead-Of-Time compilation tier and execution mode.

## 2. Syntax

```basic
JIT [ON | OFF | AUTO | BYTECODE | NATIVE | FAST | SAFE]
```

## 3. Code Example

```basic
10 REM JIT Demonstration
20 PRINT "JIT executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Program Mgmt & Editing
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | JIT |
| Category | Program Mgmt & Editing |
| Syntax | JIT [ON \| OFF \| AUTO \| BYTECODE \| NATIVE \| FAST \| SAFE] |
| Description | Configures the Just-In-Time and Ahead-Of-Time compilation tier and execution mode. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/program/stmt_jit.c |
