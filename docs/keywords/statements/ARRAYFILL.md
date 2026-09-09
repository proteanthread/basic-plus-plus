<!--
Title:        ARRAYFILL
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/assignment/arrayfill.c
Generated:    no, hand-written
Status:       current
-->

# `ARRAYFILL` Keyword Reference

## Source Header

```c
// FILENAME: arrayfill.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (arrayfill.h, eval.h, eval.c, lexer.h, lexer.c, string.c)
// NEEDS: libengine (vm.h)
// Provides runtime implementation for the ARRAYFILL statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Fills all elements of the specified array with the given value.

## 2. Syntax

```basic
ARRAYFILL array_name(), fill_value
```

## 3. Code Example

```basic
10 Val = ARRAYFILL array_name(), fill_value
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error, Error 9: Subscript out of range, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Arrays & Matrices
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | ARRAYFILL |
| Category | Arrays & Matrices |
| Syntax | ARRAYFILL array_name(), fill_value |
| Description | Fills all elements of the specified array with the given value. |
| Error Summary | Error 2: Syntax Error, Error 9: Subscript out of range, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/assignment/arrayfill.c |
