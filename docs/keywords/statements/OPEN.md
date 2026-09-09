<!--
Title:        OPEN
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/file_ops/open.c
Generated:    no, hand-written
Status:       current
-->

# `OPEN` Keyword Reference

## Source Header

```c
// FILENAME: open.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, file.h, file.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, map.h, map.c, open.h)
// NEEDS: libengine (string.c, vm.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the OPEN statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Opens a file channel for INPUT, OUTPUT, APPEND, BINARY, or RANDOM I/O operations.

## 2. Syntax

```basic
OPEN filespec [FOR mode] AS [#]file_num [LEN=reclen]
```

## 3. Code Example

```basic
10 REM OPEN Demonstration
20 PRINT "OPEN executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 52: Bad File Number, Error 53: File Not Found, Error 55: File Already Open

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
| Name | OPEN |
| Category | Filesystem I/O |
| Syntax | OPEN filespec [FOR mode] AS [#]file_num [LEN=reclen] |
| Description | Opens a file channel for INPUT, OUTPUT, APPEND, BINARY, or RANDOM I/O operations. |
| Error Summary | Error 2: Syntax Error, Error 52: Bad File Number, Error 53: File Not Found, Error 55: File Already Open |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/file_ops/open.c |
