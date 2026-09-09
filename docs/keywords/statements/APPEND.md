<!--
Title:        APPEND
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/file_ops/append.c
Generated:    no, hand-written
Status:       current
-->

# `APPEND` Keyword Reference

## Source Header

```c
// FILENAME: append.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, file.h, file.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (append.h, eval.h, eval.c, string.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the APPEND statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Opens file channel in append mode, or streams/appends program lines in memory to the end/merged positions of the disk file.

## 2. Syntax

```basic
APPEND [#]channel, "filespec$"
```

## 3. Code Example

```basic
10 REM APPEND Demonstration
20 PRINT "APPEND executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 52: Bad File Number, Error 53: File Not Found, Error 55: File Already Open, Error 70: Permission Denied

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
| Name | APPEND |
| Category | Filesystem I/O |
| Syntax | APPEND [#]channel, "filespec$" |
| Description | Opens file channel in append mode, or streams/appends program lines in memory to the end/merged positions of the disk file. |
| Error Summary | Error 2: Syntax Error, Error 52: Bad File Number, Error 53: File Not Found, Error 55: File Already Open, Error 70: Permission Denied |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/file_ops/append.c |
