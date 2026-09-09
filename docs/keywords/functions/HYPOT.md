<!--
Title:        HYPOT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/trig/hypot.c
Generated:    no, hand-written
Status:       current
-->

# `HYPOT` Keyword Reference

## Source Header

```c
// FILENAME: hypot.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (hypot.h, math.c, string.c)
// Provides runtime implementation for the HYPOT built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the Euclidean norm runtime_sqrt(sum of squares) of arguments (supports dual prefix & infix notation).

## 2. Syntax

```basic
HYPOT(val1, val2 [, ...]) or val1 HYPOT val2
```

## 3. Code Example

```basic
10 Val = HYPOT(val1, val2 [, ...]) or val1 HYPOT val2
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (HYPOT expects numeric arguments)

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
| Name | HYPOT |
| Category | Math Functions |
| Syntax | HYPOT(val1, val2 [, ...]) or val1 HYPOT val2 |
| Description | Returns the Euclidean norm runtime_sqrt(sum of squares) of arguments (supports dual prefix & infix notation). |
| Error Summary | Error 13: Type Mismatch (HYPOT expects numeric arguments) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/trig/hypot.c |
