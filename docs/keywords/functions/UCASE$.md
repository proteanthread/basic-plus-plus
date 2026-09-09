<!--
Title:        UCASE$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/ucase.c
Generated:    no, hand-written
Status:       current
-->

# `UCASE$` Keyword Reference

## Source Header

```c
// FILENAME: ucase.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (string.c, ucase.h)
// Provides runtime implementation for the UCASE built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns a copy of str$ with all lowercase letters converted to uppercase.

## 2. Syntax

```basic
UCASE$(str$)
```

## 3. Code Example

```basic
10 Val = UCASE$(str$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (UCASE$ expects one string argument)

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
| Name | UCASE$ |
| Category | String Functions |
| Syntax | UCASE$(str$) |
| Description | Returns a copy of str$ with all lowercase letters converted to uppercase. |
| Error Summary | Error 13: Type Mismatch (UCASE$ expects one string argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/ucase.c |
