<!--
Title:        NAME
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/dir_ops/name.c
Generated:    no, hand-written
Status:       current
-->

# `NAME` Keyword Reference

## Source Header

```c
// FILENAME: name.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, name.h, string.c, vm.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the NAME statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Renames an existing disk file or directory.

## 2. Syntax

```basic
NAME oldspec AS newspec
```

## 3. Code Example

```basic
10 REM NAME Demonstration
20 PRINT "NAME executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 53: File Not Found, Error 58: File Already Exists

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
| Name | NAME |
| Category | Filesystem I/O |
| Syntax | NAME oldspec AS newspec |
| Description | Renames an existing disk file or directory. |
| Error Summary | Error 2: Syntax Error, Error 53: File Not Found, Error 58: File Already Exists |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/dir_ops/name.c |
