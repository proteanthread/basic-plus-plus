<!--
Title:        COS
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/trig/cos.c
Generated:    no, hand-written
Status:       current
-->

# `COS` Keyword Reference

## Source Header

```c
// FILENAME: runtime_cos.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (runtime_cos.h, math.c, string.c)
// Provides runtime implementation for the COS built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the trigonometric cosine of an angle (radians by default, degrees or grads if DEGREE/GRAD mode).

## 2. Syntax

```basic
COS(angle)
```

## 3. Code Example

```basic
10 Angle = 0.5
20 PRINT "COS("; Angle; ") = "; COS(Angle)
```

## 4. Error Conditions

Error 13: Type Mismatch (COS expects one numeric argument)

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
| Name | COS |
| Category | Math Functions |
| Syntax | COS(angle) |
| Description | Returns the trigonometric cosine of an angle (radians by default, degrees or grads if DEGREE/GRAD mode). |
| Error Summary | Error 13: Type Mismatch (COS expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/trig/cos.c |
