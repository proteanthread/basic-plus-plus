<!--
Title:        LERP
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/algebra/lerp.c
Generated:    no, hand-written
Status:       current
-->

# `LERP` Keyword Reference

## Source Header

```c
// FILENAME: lerp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (lerp.h, math.c, string.c)
// Provides runtime implementation for the LERP built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Performs linear interpolation between a and b using weight t (a + (b - a) * t).

## 2. Syntax

```basic
LERP(a, b, t)
```

## 3. Code Example

```basic
10 Val = LERP(a, b, t)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (LERP expects three numeric arguments)

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
| Name | LERP |
| Category | Math Functions |
| Syntax | LERP(a, b, t) |
| Description | Performs linear interpolation between a and b using weight t (a + (b - a) * t). |
| Error Summary | Error 13: Type Mismatch (LERP expects three numeric arguments) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/algebra/lerp.c |
