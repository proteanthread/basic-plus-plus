<!--
Title:        TIMEVALUE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/datetime/timevalue.c
Generated:    no, hand-written
Status:       current
-->

# `TIMEVALUE` Keyword Reference

## Source Header

```c
// FILENAME: timevalue.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (datetime_common.h, timevalue.h)
// Provides runtime implementation for the TIMEVALUE function in BASIC++.
```

## 1. Description & Usage

Parses time string and returns fractional Microsoft Serial Time number.

## 2. Syntax

```basic
TIMEVALUE(time_str$)
```

## 3. Code Example

```basic
10 Val = TIMEVALUE(time_str$)
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
| Name | TIMEVALUE |
| Category | Date and Time |
| Syntax | TIMEVALUE(time_str$) |
| Description | Parses time string and returns fractional Microsoft Serial Time number. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/datetime/timevalue.c |
