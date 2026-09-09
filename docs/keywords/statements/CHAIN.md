<!--
Title:        CHAIN
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/program/chain.c
Generated:    no, hand-written
Status:       current
-->

# `CHAIN` Keyword Reference

## Source Header

```c
// FILENAME: chain.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (chain.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the CHAIN statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Passes control to another program file with optional variable preservation and line merging.

## 2. Syntax

```basic
CHAIN [MERGE] filename_expr [, [line_number] [, ALL] [, MERGE]]
```

## 3. Code Example

```basic
10 REM CHAIN Demonstration
20 PRINT "CHAIN executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 53: File Not Found, Error 8: Undefined Line Number

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Program Mgmt & Editing
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CHAIN |
| Category | Program Mgmt & Editing |
| Syntax | CHAIN [MERGE] filename_expr [, [line_number] [, ALL] [, MERGE]] |
| Description | Passes control to another program file with optional variable preservation and line merging. |
| Error Summary | Error 2: Syntax Error, Error 53: File Not Found, Error 8: Undefined Line Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/program/chain.c |
