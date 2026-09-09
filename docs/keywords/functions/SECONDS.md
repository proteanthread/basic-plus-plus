<!--
Title:        SECONDS
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/datetime/second.c
Generated:    no, hand-written
Status:       current
-->

# `SECONDS` Keyword Reference

## Source Header

```c
// FILENAME: second.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (datetime_common.h, second.h)
// Provides runtime implementation for the SECOND function in BASIC++.
```

## 1. Description & Usage

Returns the second component (0 to 59) of the current or specified time.

## 2. Syntax

```basic
SECONDS [(time_expr)]
```

## 3. Code Example

```basic
10 Val = SECONDS [(time_e10pr)]
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
| Name | SECONDS |
| Category | Date & Time |
| Syntax | SECONDS [(time_expr)] |
| Description | Returns the second component (0 to 59) of the current or specified time. |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/eval/functions/datetime/second.c |
