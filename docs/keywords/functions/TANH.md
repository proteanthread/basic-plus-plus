<!--
Title:        TANH
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/trig/tanh.c
Generated:    no, hand-written
Status:       current
-->

# `TANH` Keyword Reference

## Source Header

```c
// FILENAME: tanh.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, string.c, tanh.h)
// Provides runtime implementation for the TANH built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the hyperbolic tangent of x.

## 2. Syntax

```basic
TANH(x) | HTN(x)
```

## 3. Code Example

```basic
10 Val = TANH(10) | HTN(10)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (TANH expects one numeric argument)

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
| Name | TANH |
| Category | Math & Trigonometry |
| Syntax | TANH(x) \| HTN(x) |
| Description | Returns the hyperbolic tangent of x. |
| Error Summary | Error 13: Type Mismatch (TANH expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/trig/tanh.c |
