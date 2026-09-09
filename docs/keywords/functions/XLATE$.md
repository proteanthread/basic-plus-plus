<!--
Title:        XLATE$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/conversion/xlate.c
Generated:    no, hand-written
Status:       current
-->

# `XLATE$` Keyword Reference

## Source Header

```c
// FILENAME: xlate.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (string.c, xlate.h)
// Provides runtime implementation for the XLATE built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Translates characters in src_str using character mapping table_str.

## 2. Syntax

```basic
XLATE$(src_str, table_str)
```

## 3. Code Example

```basic
10 Val = XLATE$(src_str, table_str)
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
| Name | XLATE$ |
| Category | String Functions |
| Syntax | XLATE$(src_str, table_str) |
| Description | Translates characters in src_str using character mapping table_str. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/conversion/xlate.c |
