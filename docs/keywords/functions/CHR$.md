<!--
Title:        CHR$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/conversion/chr.c
Generated:    no, hand-written
Status:       current
-->

# `CHR$` Keyword Reference

## Source Header

```c
// FILENAME: chr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (chr.h, string.c, vm.h)
// Provides runtime implementation for the CHR built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns a 1-character string containing the character corresponding to ASCII code (0-255).

## 2. Syntax

```basic
CHR$(code)
```

## 3. Code Example

```basic
10 Val = CHR$(code)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call (code out of range 0-255), Error 13: Type Mismatch (CHR$ expects one numeric argument)

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
| Name | CHR$ |
| Category | String Functions |
| Syntax | CHR$(code) |
| Description | Returns a 1-character string containing the character corresponding to ASCII code (0-255). |
| Error Summary | Error 5: Illegal Function Call (code out of range 0-255), Error 13: Type Mismatch (CHR$ expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/conversion/chr.c |
