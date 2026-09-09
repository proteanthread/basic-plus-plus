<!--
Title:        FORMAT$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/conversion/func_format.c
Generated:    no, hand-written
Status:       current
-->

# `FORMAT$` Keyword Reference

## Source Header

```c
// FILENAME: func_format.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libengine (func_format.h, vm.h)
// Provides runtime implementation for the FORMAT$ string formatting function 
```

## 1. Description & Usage

Formats a numeric or string value using a USING mask or format string into a new string.

## 2. Syntax

```basic
FORMAT$(val, mask$)
```

## 3. Code Example

```basic
10 FOR I = 1 TO 5
20   PRINT "Iteration: "; I
30 NEXT I
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: String Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | FORMAT$ |
| Category | String Functions |
| Syntax | FORMAT$(val, mask$) |
| Description | Formats a numeric or string value using a USING mask or format string into a new string. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/conversion/func_format.c |
