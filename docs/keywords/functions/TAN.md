<!--
Title:        TAN
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/trig/tan.c
Generated:    no, hand-written
Status:       current
-->

# `TAN` Keyword Reference

## Source Header

```c
// FILENAME: runtime_tan.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, string.c, runtime_tan.h)
// Provides runtime implementation for the TAN built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the trigonometric tangent of an angle (radians by default, degrees or grads if DEGREE/GRAD mode).

## 2. Syntax

```basic
TAN(angle)
```

## 3. Code Example

```basic
10 Val = TAN(angle)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (TAN expects one numeric argument)

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
| Name | TAN |
| Category | Math Functions |
| Syntax | TAN(angle) |
| Description | Returns the trigonometric tangent of an angle (radians by default, degrees or grads if DEGREE/GRAD mode). |
| Error Summary | Error 13: Type Mismatch (TAN expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/trig/tan.c |
