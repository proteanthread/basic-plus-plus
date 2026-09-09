<!--
Title:        FIELD
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/file_ops/field.c
Generated:    no, hand-written
Status:       current
-->

# `FIELD` Keyword Reference

## Source Header

```c
// FILENAME: field.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h, strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, field.h, lexer.h, lexer.c, string.c, vm.h
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the FIELD statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Allocates space in a random file buffer for record variables.

## 2. Syntax

```basic
FIELD [#]file_num, width AS string_var [, width AS string_var...]
```

## 3. Code Example

```basic
10 REM FIELD Demonstration
20 PRINT "FIELD executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 50: Field Overflow, Error 52: Bad File Number

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Filesystem I/O
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | FIELD |
| Category | Filesystem I/O |
| Syntax | FIELD [#]file_num, width AS string_var [, width AS string_var...] |
| Description | Allocates space in a random file buffer for record variables. |
| Error Summary | Error 2: Syntax Error, Error 50: Field Overflow, Error 52: Bad File Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/file_ops/field.c |
