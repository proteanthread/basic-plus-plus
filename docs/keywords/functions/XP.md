<!--
Title:        XP
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/algebra/xp.c
Generated:    no, hand-written
Status:       current
-->

# `XP` Keyword Reference

## Source Header

```c
// FILENAME: xp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h)
// NEEDS: libengine (xp.h)
// Provides runtime implementation for the XP (Exponent Part) function in BASI
```

## 1. Description & Usage

Returns the integer base-10 exponent of a number in scientific notation (JOSS).

## 2. Syntax

```basic
XP(num)
```

## 3. Code Example

```basic
10 Val = XP(num)
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
| Name | XP |
| Category | Math Functions |
| Syntax | XP(num) |
| Description | Returns the integer base-10 exponent of a number in scientific notation (JOSS). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/algebra/xp.c |
