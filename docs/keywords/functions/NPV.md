<!--
Title:        NPV
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/financial/npv.c
Generated:    no, hand-written
Status:       current
-->

# `NPV` Keyword Reference

## Source Header

```c
// FILENAME: npv.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h)
// NEEDS: libengine (financial_common.h, npv.h)
// Provides runtime implementation for the NPV function in BASIC++.
```

## 1. Description & Usage

Calculates the net present value of an investment based on a discount rate and periodic cash flows.

## 2. Syntax

```basic
x = NPV(rate, val1, val2, ...)
```

## 3. Code Example

```basic
10 Val = 10 = NPV(rate, val1, val2, ...)
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
| Name | NPV |
| Category | Financial |
| Syntax | x = NPV(rate, val1, val2, ...) |
| Description | Calculates the net present value of an investment based on a discount rate and periodic cash flows. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/financial/npv.c |
