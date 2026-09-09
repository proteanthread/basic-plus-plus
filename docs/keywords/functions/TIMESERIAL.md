<!--
Title:        TIMESERIAL
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/datetime/timeserial.c
Generated:    no, hand-written
Status:       current
-->

# `TIMESERIAL` Keyword Reference

## Source Header

```c
// FILENAME: timeserial.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h)
// NEEDS: libengine (timeserial.h, datetime_common.h)
// Provides runtime implementation for the TIMESERIAL function in BASIC++.
```

## 1. Description & Usage

Returns fractional Microsoft Serial Time for specified hour, minute, and second.

## 2. Syntax

```basic
TIMESERIAL(hour%, minute%, second%)
```

## 3. Code Example

```basic
10 Val = TIMESERIAL(hour%, minute%, second%)
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
| Name | TIMESERIAL |
| Category | Date and Time |
| Syntax | TIMESERIAL(hour%, minute%, second%) |
| Description | Returns fractional Microsoft Serial Time for specified hour, minute, and second. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/datetime/timeserial.c |
