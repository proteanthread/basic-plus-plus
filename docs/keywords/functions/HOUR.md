<!--
Title:        HOUR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/datetime/hour.c
Generated:    no, hand-written
Status:       current
-->

# `HOUR` Keyword Reference

## Source Header

```c
// FILENAME: hour.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (datetime_common.h, hour.h)
// Provides runtime implementation for the HOUR function in BASIC++.
```

## 1. Description & Usage

Returns hour of the day in 24-hr (HOUR) or 12-hr (HOURS) or formatted with AM/PM (HOUR$).

## 2. Syntax

```basic
HOUR[(serial#)] / HOURS / HOUR$
```

## 3. Code Example

```basic
10 Val = HOUR[(serial#)] / HOURS / HOUR$
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
| Name | HOUR |
| Category | Date and Time |
| Syntax | HOUR[(serial#)] / HOURS / HOUR$ |
| Description | Returns hour of the day in 24-hr (HOUR) or 12-hr (HOURS) or formatted with AM/PM (HOUR$). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/datetime/hour.c |
