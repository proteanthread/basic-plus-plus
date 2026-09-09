<!--
Title:        PDIF
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/algebra/pdif.c
Generated:    no, hand-written
Status:       current
-->

# `PDIF` Keyword Reference

## Source Header

```c
// FILENAME: pdif.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, pdif.h, string.c)
// Provides runtime implementation for the PDIF built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the positive difference of a and b (a - b if a > b, else 0).

## 2. Syntax

```basic
PDIF(a, b)
```

## 3. Code Example

```basic
10 Val = PDIF(a, b)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (PDIF expects two numeric arguments)

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
| Name | PDIF |
| Category | Math Functions |
| Syntax | PDIF(a, b) |
| Description | Returns the positive difference of a and b (a - b if a > b, else 0). |
| Error Summary | Error 13: Type Mismatch (PDIF expects two numeric arguments) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/algebra/pdif.c |
