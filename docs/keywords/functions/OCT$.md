<!--
Title:        OCT$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/conversion/oct.c
Generated:    no, hand-written
Status:       current
-->

# `OCT$` Keyword Reference

## Source Header

```c
// FILENAME: oct.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (oct.h, string.c)
// Provides runtime implementation for the OCT built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the octal string representation of integer x.

## 2. Syntax

```basic
OCT$(x)
```

## 3. Code Example

```basic
10 Val = OCT$(10)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (OCT$ expects one numeric argument)

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
| Name | OCT$ |
| Category | String Functions |
| Syntax | OCT$(x) |
| Description | Returns the octal string representation of integer x. |
| Error Summary | Error 13: Type Mismatch (OCT$ expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/conversion/oct.c |
