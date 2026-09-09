<!--
Title:        CREATE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/binary_ops/create.c
Generated:    no, hand-written
Status:       current
-->

# `CREATE` Keyword Reference

## Source Header

```c
// FILENAME: create.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h, strings.h, strings.c)
// NEEDS: libengine (create.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.
// NEEDS: libkernel (errors.h, vdev.h, vdev.c)
// Provides runtime implementation for the CREATE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Creates a new file on disk (Apple /// Business BASIC & BASIC09).

## 2. Syntax

```basic
CREATE filename$ [, type] | CREATE #ch, filename$ [: mode]
```

## 3. Code Example

```basic
10 REM CREATE Demonstration
20 PRINT "CREATE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 52: Bad File Number, Error 70: Permission Denied

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Filesystem
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CREATE |
| Category | Filesystem |
| Syntax | CREATE filename$ [, type] \| CREATE #ch, filename$ [: mode] |
| Description | Creates a new file on disk (Apple /// Business BASIC & BASIC09). |
| Error Summary | Error 2: Syntax Error, Error 52: Bad File Number, Error 70: Permission Denied |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/binary_ops/create.c |
