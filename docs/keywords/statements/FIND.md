<!--
Title:        FIND
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/binary_ops/find.c
Generated:    no, hand-written
Status:       current
-->

# `FIND` Keyword Reference

## Source Header

```c
// FILENAME: find.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h)
// NEEDS: libengine (eval.h, eval.c, find.h, string.c)
// NEEDS: libkernel (errors.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the FIND statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

DEC RSTS/E RMS-11 statement to position the file pointer at a specific record without data transfer.

## 2. Syntax

```basic
FIND [#]channel [, RECORD record_number]
```

## 3. Code Example

```basic
10 REM FIND Demonstration
20 PRINT "FIND executed successfully."
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
| Name | FIND |
| Category | Filesystem I/O |
| Syntax | FIND [#]channel [, RECORD record_number] |
| Description | DEC RSTS/E RMS-11 statement to position the file pointer at a specific record without data transfer. |
| Error Summary | Error 2: Syntax Error, Error 52: Bad File Number, Error 63: Bad Record Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/binary_ops/find.c |
