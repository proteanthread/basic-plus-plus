<!--
Title:        REMOVE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/introspection/remove.c
Generated:    no, hand-written
Status:       current
-->

# `REMOVE` Keyword Reference

## Source Header

```c
// FILENAME: remove.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, hal.h, memops.h, strings.h, strops.h, variables.h
// NEEDS: libengine (eval.h, remove.h, stmt.h)
// Provides runtime implementation for the REMOVE statement in BASIC++.
```

## 1. Description & Usage

In-place removal/cleansing of values or substring patterns from variables and arrays.

## 2. Syntax

```basic
REMOVE var_or_array, value | REMOVE$ str_var_or_array, target_str$
```

## 3. Code Example

```basic
10 REM REMOVE Demonstration
20 PRINT "REMOVE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 9: Subscript Out of Range, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Variables & Memory
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | REMOVE |
| Category | Variables & Memory |
| Syntax | REMOVE var_or_array, value \| REMOVE$ str_var_or_array, target_str$ |
| Description | In-place removal/cleansing of values or substring patterns from variables and arrays. |
| Error Summary | Error 2: Syntax Error, Error 9: Subscript Out of Range, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/introspection/remove.c |
