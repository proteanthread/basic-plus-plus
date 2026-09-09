<!--
Title:        COMPILE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/program/stmt_compile.c
Generated:    no, hand-written
Status:       current
-->

# `COMPILE` Keyword Reference

## Source Header

```c
// FILENAME: stmt_compile.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore, libengine, libkernel
// Implements the COMPILE statement for Ahead-Of-Time in-memory compilation.
//
// ---- Includes ----
```

## 1. Description & Usage

Pre-compiles the current program in memory Ahead-Of-Time (AOT) into JIT bytecode blocks.

## 2. Syntax

```basic
COMPILE [ALL | SUB | JIT]
```

## 3. Code Example

```basic
10 REM COMPILE Demonstration
20 PRINT "COMPILE executed successfully."
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
| Name | COMPILE |
| Category | Program Mgmt & Editing |
| Syntax | COMPILE [ALL \| SUB \| JIT] |
| Description | Pre-compiles the current program in memory Ahead-Of-Time (AOT) into JIT bytecode blocks. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/program/stmt_compile.c |
