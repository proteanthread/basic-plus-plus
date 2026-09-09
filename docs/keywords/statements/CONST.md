<!--
Title:        CONST
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/declaration/const.c
Generated:    no, hand-written
Status:       current
-->

# `CONST` Keyword Reference

## Source Header

```c
// FILENAME: const.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (const.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h
// Provides runtime implementation for the CONST statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Declares one or more symbolic constants assigned to literal or constant expressions.

## 2. Syntax

```basic
CONST constantname = expression [, constantname = expression...]
```

## 3. Code Example

```basic
10 REM CONST Demonstration
20 PRINT "CONST executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Variables & Memory
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CONST |
| Category | Variables & Memory |
| Syntax | CONST constantname = expression [, constantname = expression...] |
| Description | Declares one or more symbolic constants assigned to literal or constant expressions. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/declaration/const.c |
