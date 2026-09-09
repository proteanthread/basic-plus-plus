<!--
Title:        RAD$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/conversion/rad.c
Generated:    no, hand-written
Status:       current
-->

# `RAD$` Keyword Reference

## Source Header

```c
// FILENAME: rad.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (rad.h, string.c)
// Provides runtime implementation for the RAD built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Converts an integer to a DEC Radix-50 encoded 3-character string.

## 2. Syntax

```basic
RAD$(numeric_val)
```

## 3. Code Example

```basic
10 Val = RAD$(numeric_val)
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
| Name | RAD$ |
| Category | String Functions |
| Syntax | RAD$(numeric_val) |
| Description | Converts an integer to a DEC Radix-50 encoded 3-character string. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/conversion/rad.c |
