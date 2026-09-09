<!--
Title:        DP
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/algebra/dp.c
Generated:    no, hand-written
Status:       current
-->

# `DP` Keyword Reference

## Source Header

```c
// FILENAME: dp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h)
// NEEDS: libengine (dp.h)
// Provides runtime implementation for the DP (Digit Part) function in BASIC++
```

## 1. Description & Usage

Returns the normalized digit part (significand in [1, 10)) of a number in scientific notation (JOSS).

## 2. Syntax

```basic
DP(num)
```

## 3. Code Example

```basic
10 Val = DP(num)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Math Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DP |
| Category | Math Functions |
| Syntax | DP(num) |
| Description | Returns the normalized digit part (significand in [1, 10)) of a number in scientific notation (JOSS). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/algebra/dp.c |
