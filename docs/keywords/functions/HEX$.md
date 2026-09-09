<!--
Title:        HEX$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/conversion/hex.c
Generated:    no, hand-written
Status:       current
-->

# `HEX$` Keyword Reference

## Source Header

```c
// FILENAME: hex.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (funcreg.h, funcreg.c, hal.h, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (hex.h, string.c)
// Provides runtime implementation for the HEX built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

HEX$(x) returns hexadecimal string of number x (GW-BASIC); HEX(s$) decodes hex string to raw binary byte string (Wang 3300).

## 2. Syntax

```basic
HEX$(x) | HEX(hex_str$)
```

## 3. Code Example

```basic
10 Val = HEX$(10) | HEX(he10_str$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch, Error 5: Illegal function call

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
| Name | HEX$ |
| Category | String Functions |
| Syntax | HEX$(x) \| HEX(hex_str$) |
| Description | HEX$(x) returns hexadecimal string of number x (GW-BASIC); HEX(s$) decodes hex string to raw binary byte string (Wang 3300). |
| Error Summary | Error 13: Type Mismatch, Error 5: Illegal function call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/conversion/hex.c |
