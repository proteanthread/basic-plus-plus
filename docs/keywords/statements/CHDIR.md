<!--
Title:        CHDIR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/dir_ops/chdir.c
Generated:    no, hand-written
Status:       current
-->

# `CHDIR` Keyword Reference

## Source Header

```c
// FILENAME: chdir.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (prefix.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (chdir.h, eval.h, eval.c, string.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the CHDIR statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Changes the current working directory to the specified path.

## 2. Syntax

```basic
CHDIR pathname$
```

## 3. Code Example

```basic
10 REM CHDIR Demonstration
20 PRINT "CHDIR executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 76: Path Not Found

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
| Name | CHDIR |
| Category | Filesystem I/O |
| Syntax | CHDIR pathname$ |
| Description | Changes the current working directory to the specified path. |
| Error Summary | Error 2: Syntax Error, Error 76: Path Not Found |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/dir_ops/chdir.c |
