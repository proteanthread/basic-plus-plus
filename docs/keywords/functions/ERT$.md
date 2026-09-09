<!--
Title:        ERT$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/format/ert.c
Generated:    no, hand-written
Status:       current
-->

# `ERT$` Keyword Reference

## Source Header

```c
// FILENAME: ert.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (ert.h, string.c)
// Provides runtime implementation for the ERT built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the standard error message text corresponding to an error number (DEC BASIC-PLUS).

## 2. Syntax

```basic
ERT$(error_code)
```

## 3. Code Example

```basic
10 Val = ERT$(error_code)
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
| Name | ERT$ |
| Category | String Functions |
| Syntax | ERT$(error_code) |
| Description | Returns the standard error message text corresponding to an error number (DEC BASIC-PLUS). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/format/ert.c |
