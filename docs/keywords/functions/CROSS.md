<!--
Title:        CROSS
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/linear_algebra/cross.c
Generated:    no, hand-written
Status:       current
-->

# `CROSS` Keyword Reference

## Source Header

```c
// FILENAME: cross.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (cross.h, math.c, string.c)
// Provides runtime implementation for the CROSS built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

ECMA-116 standard function returning the cross product vector of 3D vectors u and v.

## 2. Syntax

```basic
CROSS(u, v)
```

## 3. Code Example

```basic
10 Val = CROSS(u, v)
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
| Name | CROSS |
| Category | Math Functions |
| Syntax | CROSS(u, v) |
| Description | ECMA-116 standard function returning the cross product vector of 3D vectors u and v. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/linear_algebra/cross.c |
