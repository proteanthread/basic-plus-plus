<!--
Title:        RADIANS
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/trig/radians.c
Generated:    no, hand-written
Status:       current
-->

# `RADIANS` Keyword Reference

## Source Header

```c
// FILENAME: radians.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, radians.h, string.c)
// Provides runtime implementation for the RADIANS built-in function in BASIC+
//
// ---- Includes ----
```

## 1. Description & Usage

Converts angle x from degrees to radians (ANSI Full BASIC 1987).

## 2. Syntax

```basic
RADIANS(x)
```

## 3. Code Example

```basic
10 Val = RADIANS(10)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (RADIANS expects one numeric argument)

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
| Name | RADIANS |
| Category | Math Functions |
| Syntax | RADIANS(x) |
| Description | Converts angle x from degrees to radians (ANSI Full BASIC 1987). |
| Error Summary | Error 13: Type Mismatch (RADIANS expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/trig/radians.c |
