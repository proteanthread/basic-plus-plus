<!--
Title:        COMP
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/linear_algebra/comp.c
Generated:    no, hand-written
Status:       current
-->

# `COMP` Keyword Reference

## Source Header

```c
// FILENAME: comp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (comp.h, math.c, string.c)
// Provides runtime implementation for the COMP built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Compares two numeric expressions a and b; returns -1 if a < b, 0 if a == b, and 1 if a > b.

## 2. Syntax

```basic
COMP(a, b)
```

## 3. Code Example

```basic
10 Val = COMP(a, b)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (COMP expects two numeric arguments)

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
| Name | COMP |
| Category | Math Functions |
| Syntax | COMP(a, b) |
| Description | Compares two numeric expressions a and b; returns -1 if a < b, 0 if a == b, and 1 if a > b. |
| Error Summary | Error 13: Type Mismatch (COMP expects two numeric arguments) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/linear_algebra/comp.c |
