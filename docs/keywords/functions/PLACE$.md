<!--
Title:        PLACE$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/place.c
Generated:    no, hand-written
Status:       current
-->

# `PLACE$` Keyword Reference

## Source Header

```c
// FILENAME: place.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, string_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (place.h, str_math_common.h)
// Provides runtime implementation for the PLACE$ function in BASIC++.
```

## 1. Description & Usage

Formats and scales precision of a decimal numeric string (VAX BASIC / BP2).

## 2. Syntax

```basic
PLACE$(str_val, place_flag, decimal_digits)
```

## 3. Code Example

```basic
10 Val = PLACE$(str_val, place_flag, decimal_digits)
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
| Name | PLACE$ |
| Category | String Arithmetic |
| Syntax | PLACE$(str_val, place_flag, decimal_digits) |
| Description | Formats and scales precision of a decimal numeric string (VAX BASIC / BP2). |
| Error Summary | Error 5: Illegal Function Call, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/place.c |
