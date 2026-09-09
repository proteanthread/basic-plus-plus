<!--
Title:        STR$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/conversion/str.c
Generated:    no, hand-written
Status:       current
-->

# `STR$` Keyword Reference

## Source Header

```c
// FILENAME: str.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libcore (num_format.h, num_format.c, string.h, strings.h, strings.c)
// NEEDS: libengine (str.h, string.c, vm.h)
// Provides runtime implementation for the STR built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the string representation of numeric expression x.

## 2. Syntax

```basic
STR$(x)
```

## 3. Code Example

```basic
10 Val = STR$(10)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (STR$ expects one numeric argument)

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
| Name | STR$ |
| Category | String Functions |
| Syntax | STR$(x) |
| Description | Returns the string representation of numeric expression x. |
| Error Summary | Error 13: Type Mismatch (STR$ expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/conversion/str.c |
