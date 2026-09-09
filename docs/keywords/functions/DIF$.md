<!--
Title:        DIF$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/dif.c
Generated:    no, hand-written
Status:       current
-->

# `DIF$` Keyword Reference

## Source Header

```c
// FILENAME: dif.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, string_fn.c)
// NEEDS: libcore (hal.h, language_descriptor.h, strings.h)
// NEEDS: libengine (dif.h, str_math_common.h, sum.h)
// Provides runtime implementation for the DIF$ function in BASIC++.
```

## 1. Description & Usage

Returns the exact high-precision decimal difference of str_a minus str_b (VAX BASIC / BP2).

## 2. Syntax

```basic
DIF$(str_a, str_b)
```

## 3. Code Example

```basic
10 Val = DIF$(str_a, str_b)
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
| Name | DIF$ |
| Category | String Arithmetic |
| Syntax | DIF$(str_a, str_b) |
| Description | Returns the exact high-precision decimal difference of str_a minus str_b (VAX BASIC / BP2). |
| Error Summary | Error 5: Illegal Function Call, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/dif.c |
