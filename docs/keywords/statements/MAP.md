<!--
Title:        MAP
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/core/program/map.c
Generated:    no, hand-written
Status:       current
-->

# `MAP` Keyword Reference

## Source Header

```c
// FILENAME: map.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine
// NEEDS: libcore (ctype.h, ctype.c, language_descriptor.h)
// NEEDS: libcore (string.h, strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, map.h, string.c)
// NEEDS: libkernel (errors.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the MAP statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

DEC RSTS/E BASIC-PLUS-2 statement declaring a static named buffer overlay across variables and RMS-11 file records.

## 2. Syntax

```basic
MAP (map_name) var1 [= len] [, var2 [= len] ...] | MAP (map_name) ... MAPEND
```

## 3. Code Example

```basic
10 Val = MAP (map_name) var1 [= len] [, var2 [= len] ...] | MAP (map_name) ... MAPEND
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error, Error 50: Field Overflow

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
| Name | MAP |
| Category | Variables & Memory |
| Syntax | MAP (map_name) var1 [= len] [, var2 [= len] ...] \| MAP (map_name) ... MAPEND |
| Description | DEC RSTS/E BASIC-PLUS-2 statement declaring a static named buffer overlay across variables and RMS-11 file records. |
| Error Summary | Error 2: Syntax Error, Error 50: Field Overflow |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/core/program/map.c |
