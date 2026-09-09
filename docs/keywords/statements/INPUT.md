<!--
Title:        INPUT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/core/io/input.c
Generated:    no, hand-written
Status:       current
-->

# `INPUT` Keyword Reference

## Source Header

```c
// FILENAME: input.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c, ctype.h, ctype.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c, using.h, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, stmt.h, string.c)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the INPUT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Prompts the user or reads values from console input into target variables.

## 2. Syntax

```basic
INPUT [;] ["prompt";] variable[, ...]
```

## 3. Code Example

```basic
10 REM INPUT Demonstration
20 PRINT "INPUT executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch, Error 52: Bad File Number

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Console I/O
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | INPUT |
| Category | Console I/O |
| Syntax | INPUT [;] ["prompt";] variable[, ...] |
| Description | Prompts the user or reads values from console input into target variables. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch, Error 52: Bad File Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/core/io/input.c |
