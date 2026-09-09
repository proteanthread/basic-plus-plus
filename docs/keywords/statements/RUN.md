<!--
Title:        RUN
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/program/run.c
Generated:    no, hand-written
Status:       current
-->

# `RUN` Keyword Reference

## Source Header

```c
// FILENAME: run.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, run.h, string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the RUN statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Starts execution of the program currently in memory or loads and runs a specified file.

## 2. Syntax

```basic
RUN [line_number | filename [, R]]
```

## 3. Code Example

```basic
10 REM RUN Demonstration
20 PRINT "RUN executed successfully."
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
| Name | RUN |
| Category | Program Mgmt & Editing |
| Syntax | RUN [line_number \| filename [, R]] |
| Description | Starts execution of the program currently in memory or loads and runs a specified file. |
| Error Summary | Error 2: Syntax Error, Error 53: File Not Found, Error 8: Undefined Line Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/program/run.c |
