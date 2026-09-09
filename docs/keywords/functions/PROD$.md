<!--
Title:        PROD$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/prod.c
Generated:    no, hand-written
Status:       current
-->

# `PROD$` Keyword Reference

## Source Header

```c
// FILENAME: prod.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, string_fn.c)
// NEEDS: libcore (hal.h, language_descriptor.h, strings.h)
// NEEDS: libengine (prod.h, str_math_common.h)
// Provides runtime implementation for the PROD$ function in BASIC++.
```

## 1. Description & Usage

Returns the exact high-precision decimal product of two numeric strings (VAX BASIC / BP2).

## 2. Syntax

```basic
PROD$(str_a, str_b [, precision])
```

## 3. Code Example

```basic
10 Val = PROD$(str_a, str_b [, precision])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: String Arithmetic
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | PROD$ |
| Category | String Arithmetic |
| Syntax | PROD$(str_a, str_b [, precision]) |
| Description | Returns the exact high-precision decimal product of two numeric strings (VAX BASIC / BP2). |
| Error Summary | Error 5: Illegal Function Call, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/prod.c |
