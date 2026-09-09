<!--
Title:        VAL
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/conversion/val.c
Generated:    no, hand-written
Status:       current
-->

# `VAL` Keyword Reference

## Source Header

```c
// FILENAME: val.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (string.c, val.h)
// Provides runtime implementation for the VAL built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the numeric value represented by string str$. Returns 0 if str$ is not a valid number.

## 2. Syntax

```basic
VAL(str$)
```

## 3. Code Example

```basic
10 Val = VAL(str$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (VAL expects one string argument)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: String Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | VAL |
| Category | String Functions |
| Syntax | VAL(str$) |
| Description | Returns the numeric value represented by string str$. Returns 0 if str$ is not a valid number. |
| Error Summary | Error 13: Type Mismatch (VAL expects one string argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/conversion/val.c |
