<!--
Title:        COT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/trig/cot.c
Generated:    no, hand-written
Status:       current
-->

# `COT` Keyword Reference

## Source Header

```c
// FILENAME: cot.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (cot.h, math.c, string.c)
// Provides runtime implementation for the COT built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the cotangent of angle x in radians (1 / runtime_tan(x)).

## 2. Syntax

```basic
COT(x)
```

## 3. Code Example

```basic
10 Val = COT(10)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 11: Division by Zero (runtime_tan(x) == 0), Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Math & Trigonometry
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | COT |
| Category | Math & Trigonometry |
| Syntax | COT(x) |
| Description | Returns the cotangent of angle x in radians (1 / runtime_tan(x)). |
| Error Summary | Error 11: Division by Zero (runtime_tan(x) == 0), Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/trig/cot.c |
