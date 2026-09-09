<!--
Title:        MKDIR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/dir_ops/mkdir.c
Generated:    no, hand-written
Status:       current
-->

# `MKDIR` Keyword Reference

## Source Header

```c
// FILENAME: mkdir.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, mkdir.h, string.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the MKDIR statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Creates a new directory on disk.

## 2. Syntax

```basic
MKDIR pathname$
```

## 3. Code Example

```basic
10 REM MKDIR Demonstration
20 PRINT "MKDIR executed successfully."
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
| Name | MKDIR |
| Category | Filesystem I/O |
| Syntax | MKDIR pathname$ |
| Description | Creates a new directory on disk. |
| Error Summary | Error 2: Syntax Error, Error 75: Path/File Access Error, Error 76: Path Not Found |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/dir_ops/mkdir.c |
