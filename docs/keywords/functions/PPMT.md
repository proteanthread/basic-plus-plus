<!--
Title:        PPMT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/financial/ppmt.c
Generated:    no, hand-written
Status:       current
-->

# `PPMT` Keyword Reference

## Source Header

```c
// FILENAME: ppmt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h)
// NEEDS: libengine (financial_common.h, ipmt.h, pmt.h, ppmt.h)
// Provides runtime implementation for the PPMT function in BASIC++.
```

## 1. Description & Usage

Calculates the payment on the principal for an investment for a given period.

## 2. Syntax

```basic
x = PPMT(rate, per, nper, pv [, fv [, type]])
```

## 3. Code Example

```basic
10 Val = 10 = PPMT(rate, per, nper, pv [, fv [, type]])
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
| Name | PPMT |
| Category | Financial |
| Syntax | x = PPMT(rate, per, nper, pv [, fv [, type]]) |
| Description | Calculates the payment on the principal for an investment for a given period. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/financial/ppmt.c |
