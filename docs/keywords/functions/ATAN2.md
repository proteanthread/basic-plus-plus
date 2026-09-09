<!--
Title:        ATAN2
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/trig/atan2.c
Generated:    no, hand-written
Status:       current
-->

# `ATAN2` Keyword Reference

## Source Header

```c
// FILENAME: runtime_atan2.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (runtime_atan2.h, math.c, string.c)
// Provides runtime implementation for the ATAN2 built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the 2-argument arctangent of y and x in radians.

## 2. Syntax

```basic
ATAN2(y, x)
```

## 3. Code Example

```basic
10 Val = ATAN2(y, 10)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (ATAN2 expects two numeric arguments)

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
| Name | ATAN2 |
| Category | Math Functions |
| Syntax | ATAN2(y, x) |
| Description | Returns the 2-argument arctangent of y and x in radians. |
| Error Summary | Error 13: Type Mismatch (ATAN2 expects two numeric arguments) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/trig/atan2.c |
