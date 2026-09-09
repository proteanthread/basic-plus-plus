<!--
Title:        ERASE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/data/erase.c
Generated:    no, hand-written
Status:       current
-->

# `ERASE` Keyword Reference

## Source Header

```c
// FILENAME: erase.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (erase.h, string.c)
// Provides runtime implementation for the ERASE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Eliminates dynamic arrays from memory and reallocates storage space.

## 2. Syntax

```basic
ERASE array_name1 [, array_name2...]
```

## 3. Code Example

```basic
10 REM ERASE Demonstration
20 PRINT "ERASE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 10: Array Not Dimensioned

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Variables & Memory
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | ERASE |
| Category | Variables & Memory |
| Syntax | ERASE array_name1 [, array_name2...] |
| Description | Eliminates dynamic arrays from memory and reallocates storage space. |
| Error Summary | Error 2: Syntax Error, Error 10: Array Not Dimensioned |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/data/erase.c |
