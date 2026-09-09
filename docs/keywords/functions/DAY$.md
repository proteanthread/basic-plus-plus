<!--
Title:        DAY$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/datetime/day.c
Generated:    no, hand-written
Status:       current
-->

# `DAY$` Keyword Reference

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

Returns the weekday name string (e.g. 'Monday') for current or specified date.

## 2. Syntax

```basic
DAY$ [(date_expr)]
```

## 3. Code Example

```basic
10 Val = DAY$ [(date_e10pr)]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Date & Time
- **Subsystem**: SUBSYSTEM_PLATFORM
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DAY$ |
| Category | Date & Time |
| Syntax | DAY$ [(date_expr)] |
| Description | Returns the weekday name string (e.g. 'Monday') for current or specified date. |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/eval/functions/datetime/day.c |
