<!--
Title:        LOG2
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/algebra/log2.c
Generated:    no, hand-written
Status:       current
-->

# `LOG2` Keyword Reference

## Source Header

```c
// FILENAME: log2.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (log2.h, math.c, string.c)
// Provides runtime implementation for the LOG2 built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the base-2 logarithm of x (x > 0).

## 2. Syntax

```basic
LOG2(x)
```

## 3. Code Example

```basic
10 Val = LOG2(10)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call (x <= 0), Error 13: Type Mismatch (LOG2 expects one numeric argument)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Math & Trigonometry
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | LOG2 |
| Category | Math & Trigonometry |
| Syntax | LOG2(x) |
| Description | Returns the base-2 logarithm of x (x > 0). |
| Error Summary | Error 5: Illegal Function Call (x <= 0), Error 13: Type Mismatch (LOG2 expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/algebra/log2.c |
