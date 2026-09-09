<!--
Title:        RND
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/random/rnd.c
Generated:    no, hand-written
Status:       current
-->

# `RND` Keyword Reference

## Source Header

```c
// FILENAME: rnd.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, rnd.h, string.c)
// Provides runtime implementation for the RND built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns a double-precision pseudo-random number in range [0.0, 1.0). If x < 0, seeds generator.

## 2. Syntax

```basic
RND[(x)]
```

## 3. Code Example

```basic
10 Val = RND[(10)]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (non-numeric argument)

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
| Name | RND |
| Category | Math Functions |
| Syntax | RND[(x)] |
| Description | Returns a double-precision pseudo-random number in range [0.0, 1.0). If x < 0, seeds generator. |
| Error Summary | Error 13: Type Mismatch (non-numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/random/rnd.c |
