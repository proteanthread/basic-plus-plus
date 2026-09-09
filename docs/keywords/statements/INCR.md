<!--
Title:        INCR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/assignment/incr.c
Generated:    no, hand-written
Status:       current
-->

# `INCR` Keyword Reference

## Source Header

```c
// FILENAME: incr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (arrays.h, language_descriptor.h, strings.h, variables.h)
// NEEDS: libengine (eval.h, incr.h, lexer.h, vm.h)
// Provides runtime implementation for the INCR statement in BASIC++.
```

## 1. Description & Usage

Increments the numeric variable or array element by step (default 1).

## 2. Syntax

```basic
INCR variable [, step]
```

## 3. Code Example

```basic
10 REM INCR Demonstration
20 PRINT "INCR executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 9: Subscript out of range, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Variables & Memory
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | INCR |
| Category | Variables & Memory |
| Syntax | INCR variable [, step] |
| Description | Increments the numeric variable or array element by step (default 1). |
| Error Summary | Error 2: Syntax Error, Error 9: Subscript out of range, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/assignment/incr.c |
