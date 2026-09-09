<!--
Title:        LBOUND
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/linear_algebra/lbound.c
Generated:    no, hand-written
Status:       current
-->

# `LBOUND` Keyword Reference

## Source Header

```c
// FILENAME: lbound.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (arrays.h, arrays.c, math.h)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (lbound.h, math.c, string.c, vm.h)
// Provides runtime implementation for the LBOUND built-in function in BASIC++
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the lowest subscript for the indicated dimension of an array.

## 2. Syntax

```basic
low% = LBOUND(array [, dimension%])
```

## 3. Code Example

```basic
10 Val = low% = LBOUND(array [, dimension%])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 9: Subscript out of range, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Array Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | LBOUND |
| Category | Array Functions |
| Syntax | low% = LBOUND(array [, dimension%]) |
| Description | Returns the lowest subscript for the indicated dimension of an array. |
| Error Summary | Error 9: Subscript out of range, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/linear_algebra/lbound.c |
