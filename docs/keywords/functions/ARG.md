<!--
Title:        ARG
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/trig/arg.c
Generated:    no, hand-written
Status:       current
-->

# `ARG` Keyword Reference

## Source Header

```c
// FILENAME: arg.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (arg.h)
// Provides runtime implementation for the ARG function (JOSS / RAND P-2922).
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the polar angle (argument) of coordinate (x, y) in radians (JOSS / RAND P-2922).

## 2. Syntax

```basic
ARG(x, y)
```

## 3. Code Example

```basic
10 Val = ARG(10, y)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

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
| Name | ARG |
| Category | Math Functions |
| Syntax | ARG(x, y) |
| Description | Returns the polar angle (argument) of coordinate (x, y) in radians (JOSS / RAND P-2922). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/trig/arg.c |
