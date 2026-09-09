<!--
Title:        CLOSE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/file_ops/close.c
Generated:    no, hand-written
Status:       current
-->

# `CLOSE` Keyword Reference

## Source Header

```c
// FILENAME: close.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h)
// NEEDS: libengine (close.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h
// Provides runtime implementation for the CLOSE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Closes open file channels, flushing pending I/O buffers to disk.

## 2. Syntax

```basic
CLOSE [[#]file_num1[, [#]file_num2...]]
```

## 3. Code Example

```basic
10 REM CLOSE Demonstration
20 PRINT "CLOSE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 52: Bad File Number

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
| Name | CLOSE |
| Category | Filesystem I/O |
| Syntax | CLOSE [[#]file_num1[, [#]file_num2...]] |
| Description | Closes open file channels, flushing pending I/O buffers to disk. |
| Error Summary | Error 2: Syntax Error, Error 52: Bad File Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/file_ops/close.c |
