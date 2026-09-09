<!--
Title:        LOCK
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/file_ops/lock.c
Generated:    no, hand-written
Status:       current
-->

# `LOCK` Keyword Reference

## Source Header

```c
// FILENAME: lock.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, lock.h, string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the LOCK statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Locks a file on disk (Apple /// Business BASIC) or locks a file channel/record range (QuickBASIC).

## 2. Syntax

```basic
LOCK filepath$ | LOCK [#]file_num [, [record_start] [TO record_end]]
```

## 3. Code Example

```basic
10 REM LOCK Demonstration
20 PRINT "LOCK executed successfully."
```

## 4. Error Conditions

Error 52: Bad File Number, Error 53: File Not Found, Error 70: Permission Denied

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: File & Resource Locking
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | LOCK |
| Category | File & Resource Locking |
| Syntax | LOCK filepath$ \| LOCK [#]file_num [, [record_start] [TO record_end]] |
| Description | Locks a file on disk (Apple /// Business BASIC) or locks a file channel/record range (QuickBASIC). |
| Error Summary | Error 52: Bad File Number, Error 53: File Not Found, Error 70: Permission Denied |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/file_ops/lock.c |
