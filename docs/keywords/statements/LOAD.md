<!--
Title:        LOAD
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/program/load.c
Generated:    no, hand-written
Status:       current
-->

# `LOAD` Keyword Reference

## Source Header

```c
// FILENAME: load.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, load.h, string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the LOAD statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Loads a program file into memory from disk storage, optionally running it.

## 2. Syntax

```basic
LOAD filename_expr [, R]
```

## 3. Code Example

```basic
10 REM LOAD Demonstration
20 PRINT "LOAD executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 53: File Not Found, Error 70: Permission Denied

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
| Name | LOAD |
| Category | Program Mgmt & Editing |
| Syntax | LOAD filename_expr [, R] |
| Description | Loads a program file into memory from disk storage, optionally running it. |
| Error Summary | Error 2: Syntax Error, Error 53: File Not Found, Error 70: Permission Denied |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/program/load.c |
