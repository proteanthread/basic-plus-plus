<!--
Title:        ANGLE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/trig/angle.c
Generated:    no, hand-written
Status:       current
-->

# `ANGLE` Keyword Reference

## Source Header

```c
// FILENAME: angle.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (angle.h, math.c, string.c)
// Provides runtime implementation for the ANGLE built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns counterclockwise angle in radians from positive x-axis to (x, y) in [0, 2*pi) (ANSI Full BASIC 1987).

## 2. Syntax

```basic
ANGLE(x, y)
```

## 3. Code Example

```basic
10 Val = ANGLE(10, y)
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
| Name | ANGLE |
| Category | Math Functions |
| Syntax | ANGLE(x, y) |
| Description | Returns counterclockwise angle in radians from positive x-axis to (x, y) in [0, 2*pi) (ANSI Full BASIC 1987). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/trig/angle.c |
