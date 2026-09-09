<!--
Title:        COUNT$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/pick.c
Generated:    no, hand-written
Status:       current
-->

# `COUNT$` Keyword Reference

## Source Header

```c
// FILENAME: pick.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (pick.h, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for standard Pick built-in functions in BAS
//
// ---- Includes ----
```

## 1. Description & Usage

Returns element count of string array or field count of delimited dynamic string.

## 2. Syntax

```basic
COUNT$(arr$ [, match$]) | COUNT$(dyn$ [, delim$])
```

## 3. Code Example

```basic
10 Val = COUNT$(arr$ [, match$]) | COUNT$(dyn$ [, delim$])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Array & String Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | COUNT$ |
| Category | Array & String Functions |
| Syntax | COUNT$(arr$ [, match$]) \| COUNT$(dyn$ [, delim$]) |
| Description | Returns element count of string array or field count of delimited dynamic string. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/pick.c |
