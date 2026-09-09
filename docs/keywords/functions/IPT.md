<!--
Title:        IPT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/algebra/ipt.c
Generated:    no, hand-written
Status:       current
-->

# `IPT` Keyword Reference

## Source Header

```c
// FILENAME: ipt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h, string.h)
// NEEDS: libengine (ipt.h)
// Provides runtime implementation for the IPT (Integer Part) function in BASI
```

## 1. Description & Usage

Returns the integer part of a number (Business BASIC / BBx).

## 2. Syntax

```basic
IPT(num) / IP(num)
```

## 3. Code Example

```basic
10 Val = IPT(num) / IP(num)
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
| Name | IPT |
| Category | Math Functions |
| Syntax | IPT(num) / IP(num) |
| Description | Returns the integer part of a number (Business BASIC / BBx). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/algebra/ipt.c |
