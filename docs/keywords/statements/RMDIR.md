<!--
Title:        RMDIR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/dir_ops/rmdir.c
Generated:    no, hand-written
Status:       current
-->

# `RMDIR` Keyword Reference

## Source Header

```c
// FILENAME: rmdir.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, rmdir.h, string.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the RMDIR statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Removes an empty directory from disk.

## 2. Syntax

```basic
RMDIR pathname$
```

## 3. Code Example

```basic
10 REM RMDIR Demonstration
20 PRINT "RMDIR executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 75: Path/File Access Error, Error 76: Path Not Found

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
| Name | RMDIR |
| Category | Filesystem I/O |
| Syntax | RMDIR pathname$ |
| Description | Removes an empty directory from disk. |
| Error Summary | Error 2: Syntax Error, Error 75: Path/File Access Error, Error 76: Path Not Found |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/dir_ops/rmdir.c |
