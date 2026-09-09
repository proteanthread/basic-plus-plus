<!--
Title:        MAG
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/algebra/mag.c
Generated:    no, hand-written
Status:       current
-->

# `MAG` Keyword Reference

## Source Header

```c
// FILENAME: mag.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (mag.h, math.c, string.c)
// Provides runtime implementation for the MAG built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the vector magnitude (hypotenuse) or absolute magnitude/modulus.

## 2. Syntax

```basic
MAG(x, y) | MAG(complex_z) | MAG(x)
```

## 3. Code Example

```basic
10 Val = MAG(10, y) | MAG(comple10_z) | MAG(10)
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
| Name | MAG |
| Category | Math Functions |
| Syntax | MAG(x, y) \| MAG(complex_z) \| MAG(x) |
| Description | Returns the vector magnitude (hypotenuse) or absolute magnitude/modulus. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/algebra/mag.c |
