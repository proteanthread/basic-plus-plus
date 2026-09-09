<!--
Title:        COSH
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/trig/cosh.c
Generated:    no, hand-written
Status:       current
-->

# `COSH` Keyword Reference

## Source Header

```c
// FILENAME: cosh.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (cosh.h, math.c, string.c)
// Provides runtime implementation for the COSH built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the hyperbolic cosine of x.

## 2. Syntax

```basic
COSH(x) | HCS(x)
```

## 3. Code Example

```basic
10 Angle = 0.5
20 PRINT "COS("; Angle; ") = "; COS(Angle)
```

## 4. Error Conditions

Error 13: Type Mismatch (COSH expects one numeric argument)

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
| Name | COSH |
| Category | Math & Trigonometry |
| Syntax | COSH(x) \| HCS(x) |
| Description | Returns the hyperbolic cosine of x. |
| Error Summary | Error 13: Type Mismatch (COSH expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/trig/cosh.c |
