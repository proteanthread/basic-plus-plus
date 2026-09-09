<!--
Title:        MINUTE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/datetime/minute.c
Generated:    no, hand-written
Status:       current
-->

# `MINUTE` Keyword Reference

## Source Header

```c
// FILENAME: minute.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (datetime_common.h, minute.h)
// Provides runtime implementation for the MINUTE function in BASIC++.
```

## 1. Description & Usage

Returns minute of the hour as integer (MINUTE), fractional (MINUTES), or formatted string (MINUTE$).

## 2. Syntax

```basic
MINUTE[(serial#)] / MINUTES / MINUTE$
```

## 3. Code Example

```basic
10 Val = MINUTE[(serial#)] / MINUTES / MINUTE$
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
| Name | MINUTE |
| Category | Date and Time |
| Syntax | MINUTE[(serial#)] / MINUTES / MINUTE$ |
| Description | Returns minute of the hour as integer (MINUTE), fractional (MINUTES), or formatted string (MINUTE$). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/datetime/minute.c |
