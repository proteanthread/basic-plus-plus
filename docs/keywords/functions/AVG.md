<!--
Title:        AVG
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/algebra/avg.c
Generated:    no, hand-written
Status:       current
-->

# `AVG` Keyword Reference

## Source Header

```c
// FILENAME: avg.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (avg.h)
// Provides runtime implementation for the AVG / MEAN built-in function in BAS
```

## 1. Description & Usage

Returns the arithmetic mean of numeric arguments, set elements, or an array (aliases: MEAN).

## 2. Syntax

```basic
AVG(val1, val2 [, ...]) or AVG(arr) or AVG{...} or AVG[arr]
```

## 3. Code Example

```basic
10 Val = AVG(val1, val2 [, ...]) or AVG(arr) or AVG{...} or AVG[arr]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Math & Statistics Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | AVG |
| Category | Math & Statistics Functions |
| Syntax | AVG(val1, val2 [, ...]) or AVG(arr) or AVG{...} or AVG[arr] |
| Description | Returns the arithmetic mean of numeric arguments, set elements, or an array (aliases: MEAN). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/algebra/avg.c |
