<!--
Title:        EXP
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/algebra/exp.c
Generated:    no, hand-written
Status:       current
-->

# `EXP` Keyword Reference

## Source Header

```c
// FILENAME: runtime_exp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (runtime_exp.h, math.c, string.c)
// Provides runtime implementation for the EXP built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns e raised to the power of a numeric expression x.

## 2. Syntax

```basic
EXP(x)
```

## 3. Code Example

```basic
10 Val = EXP(10)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 6: Overflow (EXP exponent too large), Error 13: Type Mismatch (EXP expects one numeric argument)

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
| Name | EXP |
| Category | Math Functions |
| Syntax | EXP(x) |
| Description | Returns e raised to the power of a numeric expression x. |
| Error Summary | Error 6: Overflow (EXP exponent too large), Error 13: Type Mismatch (EXP expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/algebra/exp.c |
