<!--
Title:        FV
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/financial/fv.c
Generated:    no, hand-written
Status:       current
-->

# `FV` Keyword Reference

## Source Header

```c
// FILENAME: fv.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h)
// NEEDS: libengine (financial_common.h, fv.h)
// Provides runtime implementation for the FV function in BASIC++.
```

## 1. Description & Usage

Calculates the future value of an investment based on periodic, constant payments.

## 2. Syntax

```basic
x = FV(rate, nper, pmt [, pv [, type]])
```

## 3. Code Example

```basic
10 Val = 10 = FV(rate, nper, pmt [, pv [, type]])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Financial
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | FV |
| Category | Financial |
| Syntax | x = FV(rate, nper, pmt [, pv [, type]]) |
| Description | Calculates the future value of an investment based on periodic, constant payments. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/financial/fv.c |
