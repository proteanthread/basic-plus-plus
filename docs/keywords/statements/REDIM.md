<!--
Title:        REDIM
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/declaration/redim.c
Generated:    no, hand-written
Status:       current
-->

# `REDIM` Keyword Reference

## Source Header

```c
// FILENAME: redim.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (dim.h, dim.c, redim.h, string.c)
// Provides runtime implementation for the REDIM statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Changes the dimensions and size of dynamic arrays, optionally preserving existing data.

## 2. Syntax

```basic
REDIM [PRESERVE] array_name(subscripts...)
```

## 3. Code Example

```basic
10 DIM Arr(10)
20 Arr(5) = 100
30 PRINT "Arr(5) = "; Arr(5)
```

## 4. Error Conditions

Error 2: Syntax Error, Error 9: Subscript out of range

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
| Name | REDIM |
| Category | Variables & Memory |
| Syntax | REDIM [PRESERVE] array_name(subscripts...) |
| Description | Changes the dimensions and size of dynamic arrays, optionally preserving existing data. |
| Error Summary | Error 2: Syntax Error, Error 9: Subscript out of range |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/declaration/redim.c |
