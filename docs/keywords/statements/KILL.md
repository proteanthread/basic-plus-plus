<!--
Title:        KILL
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/dir_ops/kill.c
Generated:    no, hand-written
Status:       current
-->

# `KILL` Keyword Reference

## Source Header

```c
// FILENAME: kill.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (destroy.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, kill.h, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the KILL statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Deletes specified file from disk storage.

## 2. Syntax

```basic
KILL filespec
```

## 3. Code Example

```basic
10 REM KILL Demonstration
20 PRINT "KILL executed successfully."
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
| Name | KILL |
| Category | Filesystem I/O |
| Syntax | KILL filespec |
| Description | Deletes specified file from disk storage. |
| Error Summary | Error 2: Syntax Error, Error 53: File Not Found, Error 70: Permission Denied |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/dir_ops/kill.c |
