<!--
Title:        REMAINDER
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/algebra/remainder.c
Generated:    no, hand-written
Status:       current
-->

# `REMAINDER` Keyword Reference

## Source Header

```c
// FILENAME: remainder.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, remainder.h, string.c)
// Provides runtime implementation for the REMAINDER built-in function in BASI
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the remainder of x divided by y (ANSI Full BASIC 1987).

## 2. Syntax

```basic
REMAINDER(x, y)
```

## 3. Code Example

```basic
10 Val = REMAINDER(10, y)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 11: Division by zero, Error 13: Type Mismatch

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
| Name | REMAINDER |
| Category | Math Functions |
| Syntax | REMAINDER(x, y) |
| Description | Returns the remainder of x divided by y (ANSI Full BASIC 1987). |
| Error Summary | Error 11: Division by zero, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/algebra/remainder.c |
