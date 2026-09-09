<!--
Title:        INT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/algebra/int.c
Generated:    no, hand-written
Status:       current
-->

# `INT` Keyword Reference

## Source Header

```c
// FILENAME: int.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (int.h, math.c, string.c)
// Provides runtime implementation for the INT built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the largest integer less than or equal to x (runtime_floor conversion).

## 2. Syntax

```basic
INT(x)
```

## 3. Code Example

```basic
10 Val = INT(10)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (INT expects one numeric argument)

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
| Name | INT |
| Category | Math Functions |
| Syntax | INT(x) |
| Description | Returns the largest integer less than or equal to x (runtime_floor conversion). |
| Error Summary | Error 13: Type Mismatch (INT expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/algebra/int.c |
