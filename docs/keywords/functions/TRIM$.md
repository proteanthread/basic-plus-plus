<!--
Title:        TRIM$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/trim.c
Generated:    no, hand-written
Status:       current
-->

# `TRIM$` Keyword Reference

## Source Header

```c
// FILENAME: trim.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (string.c, trim.h)
// Provides runtime implementation for the TRIM built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns a copy of str$ with both leading and trailing whitespace removed.

## 2. Syntax

```basic
TRIM$(str$)
```

## 3. Code Example

```basic
10 Val = TRIM$(str$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (TRIM$ expects one string argument)

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
| Name | TRIM$ |
| Category | String Functions |
| Syntax | TRIM$(str$) |
| Description | Returns a copy of str$ with both leading and trailing whitespace removed. |
| Error Summary | Error 13: Type Mismatch (TRIM$ expects one string argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/trim.c |
