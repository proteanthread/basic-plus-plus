<!--
Title:        DELETE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/program/delete.c
Generated:    no, hand-written
Status:       current
-->

# `DELETE` Keyword Reference

## Source Header

```c
// FILENAME: delete.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (delete.h, isam.h, isam.c, lexer.h, lexer.c, string.c, vm.
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the DELETE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Deletes specified line numbers or ranges of lines from program memory.

## 2. Syntax

```basic
DELETE [start_line] [- [end_line]]
```

## 3. Code Example

```basic
10 REM DELETE Demonstration
20 PRINT "DELETE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 8: Undefined Line Number

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
| Name | DELETE |
| Category | Program Mgmt & Editing |
| Syntax | DELETE [start_line] [- [end_line]] |
| Description | Deletes specified line numbers or ranges of lines from program memory. |
| Error Summary | Error 2: Syntax Error, Error 8: Undefined Line Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/program/delete.c |
