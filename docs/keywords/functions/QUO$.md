<!--
Title:        QUO$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/quo.c
Generated:    no, hand-written
Status:       current
-->

# `QUO$` Keyword Reference

## Source Header

```c
// FILENAME: quo.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, string_fn.c)
// NEEDS: libcore (hal.h, language_descriptor.h, strings.h)
// NEEDS: libengine (quo.h, str_math_common.h)
// Provides runtime implementation for the QUO$ function in BASIC++.
```

## 1. Description & Usage

Returns the exact high-precision decimal quotient of str_a divided by str_b (VAX BASIC / BP2).

## 2. Syntax

```basic
QUO$(str_a, str_b [, precision])
```

## 3. Code Example

```basic
10 Val = QUO$(str_a, str_b [, precision])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 11: Division by Zero, Error 13: Type Mismatch

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
| Name | QUO$ |
| Category | String Arithmetic |
| Syntax | QUO$(str_a, str_b [, precision]) |
| Description | Returns the exact high-precision decimal quotient of str_a divided by str_b (VAX BASIC / BP2). |
| Error Summary | Error 11: Division by Zero, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/quo.c |
