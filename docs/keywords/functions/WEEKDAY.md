<!--
Title:        WEEKDAY
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/datetime/weekday.c
Generated:    no, hand-written
Status:       current
-->

# `WEEKDAY` Keyword Reference

## Source Header

```c
// FILENAME: weekday.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (datetime_common.h, weekday.h)
// Provides runtime implementation for the WEEKDAY function in BASIC++.
```

## 1. Description & Usage

Returns day of the week (1=Sunday..7=Saturday) for specified Microsoft Serial Date.

## 2. Syntax

```basic
WEEKDAY(serial#)
```

## 3. Code Example

```basic
10 Val = WEEKDAY(serial#)
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
| Name | WEEKDAY |
| Category | Date and Time |
| Syntax | WEEKDAY(serial#) |
| Description | Returns day of the week (1=Sunday..7=Saturday) for specified Microsoft Serial Date. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/datetime/weekday.c |
