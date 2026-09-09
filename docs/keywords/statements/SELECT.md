<!--
Title:        SELECT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/core/program/select.c
Generated:    no, hand-written
Status:       current
-->

# `SELECT` Keyword Reference

## Source Header

```c
// FILENAME: select.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libplatform (plat_clipboard.c, plat_console.c, plat_dl.c)
// NEEDED BY: libplatform (plat_fs.c, plat_net.c, plat_regex.c, plat_sys.c)
// NEEDED BY: libplatform (plat_thread.c, plat_time.c)
// NEEDS: libcore (ctype.h, ctype.c, language_descriptor.h)
// NEEDS: libcore (string.h)
// NEEDS: libengine (eval.h, eval.c, stmt.h, string.c, vm.h)
// Provides runtime implementation for the SELECT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Executes one of several blocks of statements depending on the value of an expression.

## 2. Syntax

```basic
SELECT CASE test_expression ... CASE expression_list ... END SELECT
```

## 3. Code Example

```basic
10 REM SELECT Demonstration
20 PRINT "SELECT executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch, Error 37: SELECT Without CASE

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
| Name | SELECT |
| Category | Control Flow |
| Syntax | SELECT CASE test_expression ... CASE expression_list ... END SELECT |
| Description | Executes one of several blocks of statements depending on the value of an expression. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch, Error 37: SELECT Without CASE |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/core/program/select.c |
