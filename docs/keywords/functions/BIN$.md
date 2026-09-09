<!--
Title:        BIN$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/conversion/bin.c
Generated:    no, hand-written
Status:       current
-->

# `BIN$` Keyword Reference

## Source Header

```c
// FILENAME: bin.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (bin.h, string.c)
// Provides runtime implementation for the BIN built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the binary string representation of integer x.

## 2. Syntax

```basic
BIN$(x)
```

## 3. Code Example

```basic
10 Val = BIN$(10)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (BIN$ expects one numeric argument)

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
| Name | BIN$ |
| Category | String Functions |
| Syntax | BIN$(x) |
| Description | Returns the binary string representation of integer x. |
| Error Summary | Error 13: Type Mismatch (BIN$ expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/conversion/bin.c |
