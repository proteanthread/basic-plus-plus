<!--
Title:        ARRAY EXT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/assignment/array_ext.c
Generated:    no, hand-written
Status:       current
-->

# `ARRAY EXT` Keyword Reference

## Source Header

```c
// FILENAME: array_ext.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c, ctype.h, ctype.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, stmt.h, string.c, vm.h)
// Provides runtime implementation for the ARRAY_EXT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Provides extended high-performance array operations including sorting and filling.

## 2. Syntax

```basic
ARRAY.SORT / ARRAY.REVERSE / ARRAY.FILL
```

## 3. Code Example

```basic
10 REM ARRAY EXT Demonstration
20 PRINT "ARRAY EXT executed successfully."
```

## 4. Error Conditions

Error 9: Subscript Out of Range

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Array Extensions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | ARRAY EXT |
| Category | Array Extensions |
| Syntax | ARRAY.SORT / ARRAY.REVERSE / ARRAY.FILL |
| Description | Provides extended high-performance array operations including sorting and filling. |
| Error Summary | Error 9: Subscript Out of Range |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/assignment/array_ext.c |
