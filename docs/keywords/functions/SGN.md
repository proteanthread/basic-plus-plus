<!--
Title:        SGN
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/algebra/sgn.c
Generated:    no, hand-written
Status:       current
-->

# `SGN` Keyword Reference

## Source Header

```c
// FILENAME: sgn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, sgn.h, string.c)
// Provides runtime implementation for the SGN built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the sign of x: 1 if x > 0, 0 if x = 0, -1 if x < 0.

## 2. Syntax

```basic
SGN(x)
```

## 3. Code Example

```basic
10 Val = SGN(10)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (SGN expects one numeric argument)

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
| Name | SGN |
| Category | Math Functions |
| Syntax | SGN(x) |
| Description | Returns the sign of x: 1 if x > 0, 0 if x = 0, -1 if x < 0. |
| Error Summary | Error 13: Type Mismatch (SGN expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/algebra/sgn.c |
