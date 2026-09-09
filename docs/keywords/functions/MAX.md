<!--
Title:        MAX
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/algebra/max.c
Generated:    no, hand-written
Status:       current
-->

# `MAX` Keyword Reference

## Source Header

```c
// FILENAME: max.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, max.h, string.c)
// Provides runtime implementation for the MAX built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the maximum of two or more numeric values (supports dual prefix & infix notation).

## 2. Syntax

```basic
MAX(val1, val2 [, ...]) or val1 MAX val2
```

## 3. Code Example

```basic
10 Val = MAX(val1, val2 [, ...]) or val1 MAX val2
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (MAX expects numeric arguments)

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
| Name | MAX |
| Category | Math Functions |
| Syntax | MAX(val1, val2 [, ...]) or val1 MAX val2 |
| Description | Returns the maximum of two or more numeric values (supports dual prefix & infix notation). |
| Error Summary | Error 13: Type Mismatch (MAX expects numeric arguments) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/algebra/max.c |
