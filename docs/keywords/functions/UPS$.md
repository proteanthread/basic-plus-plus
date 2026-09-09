<!--
Title:        UPS$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/conversion/ups.c
Generated:    no, hand-written
Status:       current
-->

# `UPS$` Keyword Reference

## Source Header

```c
// FILENAME: ups.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (string.c, ups.h)
// Provides runtime implementation for the UPS built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Converts all alphabetic characters in a string to uppercase (HP 3000 TSB).

## 2. Syntax

```basic
UPS$(str_expr) | UPS(str_expr)
```

## 3. Code Example

```basic
10 Val = UPS$(str_e10pr) | UPS(str_e10pr)
20 PRINT "Result: "; Val
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
| Name | UPS$ |
| Category | String Functions |
| Syntax | UPS$(str_expr) \| UPS(str_expr) |
| Description | Converts all alphabetic characters in a string to uppercase (HP 3000 TSB). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/conversion/ups.c |
