<!--
Title:        FPT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/algebra/fpt.c
Generated:    no, hand-written
Status:       current
-->

# `FPT` Keyword Reference

## Source Header

```c
// FILENAME: fpt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h, string.h)
// NEEDS: libengine (fpt.h)
// Provides runtime implementation for the FPT (Fractional Part) function in B
```

## 1. Description & Usage

Returns the fractional part of a number (Business BASIC / BBx).

## 2. Syntax

```basic
FPT(num) / FP(num)
```

## 3. Code Example

```basic
10 Val = FPT(num) / FP(num)
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
| Name | FPT |
| Category | Math Functions |
| Syntax | FPT(num) / FP(num) |
| Description | Returns the fractional part of a number (Business BASIC / BBx). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/algebra/fpt.c |
