<!--
Title:        SECOND
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/datetime/second.c
Generated:    no, hand-written
Status:       current
-->

# `SECOND` Keyword Reference

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

Returns second of the minute as integer (SECOND), 4-decimal precision (SECONDS), or formatted string (SECOND$).

## 2. Syntax

```basic
SECOND[(serial#)] / SECONDS / SECOND$
```

## 3. Code Example

```basic
10 Val = SECOND[(serial#)] / SECONDS / SECOND$
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
| Name | SECOND |
| Category | Date and Time |
| Syntax | SECOND[(serial#)] / SECONDS / SECOND$ |
| Description | Returns second of the minute as integer (SECOND), 4-decimal precision (SECONDS), or formatted string (SECOND$). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/datetime/second.c |
