<!--
Title:        SEC
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/trig/sec.c
Generated:    no, hand-written
Status:       current
-->

# `SEC` Keyword Reference

## Source Header

```c
// FILENAME: sec.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, sec.h, string.c)
// Provides runtime implementation for the SEC built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the secant of angle x in radians (1 / runtime_cos(x)).

## 2. Syntax

```basic
SEC(x)
```

## 3. Code Example

```basic
10 Val = SEC(10)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 11: Division by Zero (runtime_cos(x) == 0), Error 13: Type Mismatch

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
| Name | SEC |
| Category | Math & Trigonometry |
| Syntax | SEC(x) |
| Description | Returns the secant of angle x in radians (1 / runtime_cos(x)). |
| Error Summary | Error 11: Division by Zero (runtime_cos(x) == 0), Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/trig/sec.c |
