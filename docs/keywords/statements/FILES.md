<!--
Title:        FILES
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/dir_ops/files.c
Generated:    no, hand-written
Status:       current
-->

# `FILES` Keyword Reference

## Source Header

```c
// FILENAME: files.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, file.h, file.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, files.h, lexer.h, lexer.c, string.c, vm.h
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the FILES statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Displays directory listing matching specified file pattern.

## 2. Syntax

```basic
FILES [filespec]
```

## 3. Code Example

```basic
10 REM FILES Demonstration
20 PRINT "FILES executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 53: File Not Found, Error 70: Permission Denied

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
| Name | FILES |
| Category | Filesystem I/O |
| Syntax | FILES [filespec] |
| Description | Displays directory listing matching specified file pattern. |
| Error Summary | Error 2: Syntax Error, Error 53: File Not Found, Error 70: Permission Denied |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/dir_ops/files.c |
