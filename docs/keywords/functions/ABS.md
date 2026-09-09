<!--
Title:        ABS
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/algebra/abs.c
Generated:    no, hand-written
Status:       current
-->

# `ABS` Keyword Reference

## Source Header

```c
// FILENAME: abs.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (abs.h, math.c, string.c)
// Provides runtime implementation for the ABS built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the absolute value of a numeric expression.

## 2. Syntax

```basic
ABS(x)
```

## 3. Code Example

```basic
10 X = -42.5
20 PRINT "Original: "; X; " Absolute: "; ABS(X)
```

## 4. Error Conditions

Error 13: Type Mismatch (ABS expects one numeric argument)

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
| Name | ABS |
| Category | Math Functions |
| Syntax | ABS(x) |
| Description | Returns the absolute value of a numeric expression. |
| Error Summary | Error 13: Type Mismatch (ABS expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/algebra/abs.c |
