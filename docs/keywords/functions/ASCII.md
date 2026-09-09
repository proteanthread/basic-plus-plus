<!--
Title:        ASCII
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/conversion/ascii_fn.c
Generated:    no, hand-written
Status:       current
-->

# `ASCII` Keyword Reference

## Source Header

```c
// FILENAME: ascii_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (ascii_fn.h, string.c)
// Provides runtime implementation for the ASCII_FN built-in function in BASIC
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the ASCII numeric value of the first character of a string (DEC PDP-11 / HP 2000).

## 2. Syntax

```basic
ASCII(str_expr) | NUM(char_expr)
```

## 3. Code Example

```basic
10 Val = ASCII(str_e10pr) | NUM(char_e10pr)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch, Error 5: Illegal Function Call

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
| Name | ASCII |
| Category | String Functions |
| Syntax | ASCII(str_expr) \| NUM(char_expr) |
| Description | Returns the ASCII numeric value of the first character of a string (DEC PDP-11 / HP 2000). |
| Error Summary | Error 13: Type Mismatch, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/conversion/ascii_fn.c |
