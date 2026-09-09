<!--
Title:        SUM
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/algebra/math_sum.c
Generated:    no, hand-written
Status:       current
-->

# `SUM` Keyword Reference

## Source Header

```c
// FILENAME: math_sum.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (math_sum.h)
// Provides runtime implementation for the mathematical SUM built-in function 
```

## 1. Description & Usage

Returns the sum of numeric arguments, set elements, or array elements (ANSI Full BASIC / Dartmouth).

## 2. Syntax

```basic
SUM(val1, val2 [, ...]) or SUM(arr) or SUM{...} or SUM[arr]
```

## 3. Code Example

```basic
10 Val = SUM(val1, val2 [, ...]) or SUM(arr) or SUM{...} or SUM[arr]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Math & Statistics Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | SUM |
| Category | Math & Statistics Functions |
| Syntax | SUM(val1, val2 [, ...]) or SUM(arr) or SUM{...} or SUM[arr] |
| Description | Returns the sum of numeric arguments, set elements, or array elements (ANSI Full BASIC / Dartmouth). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/algebra/math_sum.c |
