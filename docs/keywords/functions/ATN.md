<!--
Title:        ATN
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/trig/atn.c
Generated:    no, hand-written
Status:       current
-->

# `ATN` Keyword Reference

## Source Header

```c
// FILENAME: atn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (atn.h, math.c, string.c)
// Provides runtime implementation for the ATN built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the arctangent of a numeric expression (radians, degrees, or grads depending on angle mode).

## 2. Syntax

```basic
ATN(x)
```

## 3. Code Example

```basic
10 Val = ATN(10)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (ATN expects one numeric argument)

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
| Name | ATN |
| Category | Math Functions |
| Syntax | ATN(x) |
| Description | Returns the arctangent of a numeric expression (radians, degrees, or grads depending on angle mode). |
| Error Summary | Error 13: Type Mismatch (ATN expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/trig/atn.c |
