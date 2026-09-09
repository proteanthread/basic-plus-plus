<!--
Title:        CLAMP
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/algebra/clamp.c
Generated:    no, hand-written
Status:       current
-->

# `CLAMP` Keyword Reference

## Source Header

```c
// FILENAME: clamp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (clamp.h, math.c, string.c)
// Provides runtime implementation for the CLAMP built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Clamps a value to be within the range [min_val, max_val].

## 2. Syntax

```basic
CLAMP(val, min_val, max_val)
```

## 3. Code Example

```basic
10 Val = CLAMP(val, min_val, ma10_val)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (CLAMP expects three numeric arguments)

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
| Name | CLAMP |
| Category | Math Functions |
| Syntax | CLAMP(val, min_val, max_val) |
| Description | Clamps a value to be within the range [min_val, max_val]. |
| Error Summary | Error 13: Type Mismatch (CLAMP expects three numeric arguments) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/algebra/clamp.c |
