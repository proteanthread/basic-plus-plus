<!--
Title:        DIM
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/declaration/dim.c
Generated:    no, hand-written
Status:       current
-->

# `DIM` Keyword Reference

## Source Header

```c
// FILENAME: dim.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (redim.c, vdim.c)
// NEEDS: libcore (arrays.h, arrays.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (struct.h, struct.c, variables.h, variables.c)
// NEEDS: libengine (dim.h, eval.h, eval.c, lexer.h, lexer.c, map.h, map.c)
// NEEDS: libengine (string.c, vm.h)
// Provides runtime implementation for the DIM statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Allocates storage space for arrays, virtual arrays, fixed strings, and class instances.

## 2. Syntax

```basic
DIM [#channel,] [SHARED] [DYNAMIC | STATIC] array_name(subscripts...) [*len] [AS type [*len]] [, ...]
```

## 3. Code Example

```basic
10 DIM Arr(10)
20 Arr(5) = 100
30 PRINT "Arr(5) = "; Arr(5)
```

## 4. Error Conditions

Error 2: Syntax Error, Error 9: Subscript out of range, Error 10: Duplicate definition

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
| Name | DIM |
| Category | Variables & Memory |
| Syntax | DIM [#channel,] [SHARED] [DYNAMIC \| STATIC] array_name(subscripts...) [*len] [AS type [*len]] [, ...] |
| Description | Allocates storage space for arrays, virtual arrays, fixed strings, and class instances. |
| Error Summary | Error 2: Syntax Error, Error 9: Subscript out of range, Error 10: Duplicate definition |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/declaration/dim.c |
