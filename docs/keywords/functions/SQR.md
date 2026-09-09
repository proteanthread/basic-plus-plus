<!--
Title:        SQR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/algebra/sqr.c
Generated:    no, hand-written
Status:       current
-->

# `SQR` Keyword Reference

## Source Header

```c
// FILENAME: sqr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, sqr.h, string.c)
// Provides runtime implementation for the SQR built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the non-negative square root of a numeric expression x >= 0.

## 2. Syntax

```basic
SQR(x)
```

## 3. Code Example

```basic
10 Val = 16
20 PRINT "SQR("; Val; ") = "; SQR(Val)
```

## 4. Error Conditions

Error 5: Illegal Function Call (SQR of negative number), Error 13: Type Mismatch (SQR expects one numeric argument)

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
| Name | SQR |
| Category | Math Functions |
| Syntax | SQR(x) |
| Description | Returns the non-negative square root of a numeric expression x >= 0. |
| Error Summary | Error 5: Illegal Function Call (SQR of negative number), Error 13: Type Mismatch (SQR expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/algebra/sqr.c |
