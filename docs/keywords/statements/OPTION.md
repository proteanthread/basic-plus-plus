<!--
Title:        OPTION
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/options/option.c
Generated:    no, hand-written
Status:       current
-->

# `OPTION` Keyword Reference

## Source Header

```c
// FILENAME: option.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, option.h, string.c, vm.
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the OPTION statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Sets minimum subscript array indexing base (0 or 1) or enforces explicit variable declaration.

## 2. Syntax

```basic
OPTION BASE {0 | 1} | OPTION EXPLICIT
```

## 3. Code Example

```basic
10 REM OPTION Demonstration
20 PRINT "OPTION executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

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
| Name | OPTION |
| Category | Variables & Memory |
| Syntax | OPTION BASE {0 \| 1} \| OPTION EXPLICIT |
| Description | Sets minimum subscript array indexing base (0 or 1) or enforces explicit variable declaration. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/options/option.c |
