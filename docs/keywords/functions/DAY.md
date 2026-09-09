<!--
Title:        DAY
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/datetime/day.c
Generated:    no, hand-written
Status:       current
-->

# `DAY` Keyword Reference

## Source Header

```c
// FILENAME: day.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (datetime_common.h, day.h)
// Provides runtime implementation for the DAY function in BASIC++.
```

## 1. Description & Usage

Returns day of the month (1-31) for specified Microsoft Serial Date.

## 2. Syntax

```basic
DAY(serial#)
```

## 3. Code Example

```basic
10 Val = DAY(serial#)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Date and Time
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DAY |
| Category | Date and Time |
| Syntax | DAY(serial#) |
| Description | Returns day of the month (1-31) for specified Microsoft Serial Date. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/datetime/day.c |
