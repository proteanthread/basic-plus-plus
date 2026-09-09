<!--
Title:        ACOS
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/trig/acos.c
Generated:    no, hand-written
Status:       current
-->

# `ACOS` Keyword Reference

## Source Header

```c
// FILENAME: runtime_acos.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (runtime_acos.h, math.c, string.c)
// Provides runtime implementation for the ACOS built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the arccosine of x (in radians, degrees, or grads depending on angle mode) for -1.0 <= x <= 1.0.

## 2. Syntax

```basic
ACOS(x)
```

## 3. Code Example

```basic
10 Angle = 0.5
20 PRINT "COS("; Angle; ") = "; COS(Angle)
```

## 4. Error Conditions

Error 5: Illegal Function Call (ACOS argument out of range [-1, 1]), Error 13: Type Mismatch (ACOS expects one numeric argument)

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
| Name | ACOS |
| Category | Math Functions |
| Syntax | ACOS(x) |
| Description | Returns the arccosine of x (in radians, degrees, or grads depending on angle mode) for -1.0 <= x <= 1.0. |
| Error Summary | Error 5: Illegal Function Call (ACOS argument out of range [-1, 1]), Error 13: Type Mismatch (ACOS expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/trig/acos.c |
