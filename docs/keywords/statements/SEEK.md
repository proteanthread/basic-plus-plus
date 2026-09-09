<!--
Title:        SEEK
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/file_ops/seek.c
Generated:    no, hand-written
Status:       current
-->

# `SEEK` Keyword Reference

## Source Header

```c
// FILENAME: seek.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, seek.h, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the SEEK statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Sets the byte offset position for the next read or write operation on an open file.

## 2. Syntax

```basic
SEEK [#]file_num, position
```

## 3. Code Example

```basic
10 REM SEEK Demonstration
20 PRINT "SEEK executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 52: Bad File Number, Error 63: Bad Record Number

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
| Name | SEEK |
| Category | Filesystem I/O |
| Syntax | SEEK [#]file_num, position |
| Description | Sets the byte offset position for the next read or write operation on an open file. |
| Error Summary | Error 2: Syntax Error, Error 52: Bad File Number, Error 63: Bad Record Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/file_ops/seek.c |
