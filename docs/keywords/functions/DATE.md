<!--
Title:        DATE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/datetime/dateserial.c
Generated:    no, hand-written
Status:       current
-->

# `DATE` Keyword Reference

## Source Header

```c
// FILENAME: dateserial.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h)
// NEEDS: libengine (dateserial.h, datetime_common.h)
// Provides runtime implementation for the DATESERIAL function in BASIC++.
```

## 1. Description & Usage

Returns the current calendar date formatted according to system locale or integer epoch day.

## 2. Syntax

```basic
DATE or DATE()
```

## 3. Code Example

```basic
10 Val = DATE or DATE()
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

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
| Name | DATE |
| Category | Date & Time |
| Syntax | DATE or DATE() |
| Description | Returns the current calendar date formatted according to system locale or integer epoch day. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/eval/functions/datetime/dateserial.c |
