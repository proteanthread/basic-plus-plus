<!--
Title:        PRINT#
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/file_ops/print_file.c
Generated:    no, hand-written
Status:       current
-->

# `PRINT#` Keyword Reference

## Source Header

```c
// FILENAME: print_file.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (num_format.h, num_format.c, string.h, strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, print_file.h, string.c)
// NEEDS: libengine (vm.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the PRINT_FILE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Writes sequential formatted text data to an open disk file channel.

## 2. Syntax

```basic
PRINT #file_num, expression_list
```

## 3. Code Example

```basic
10 REM PRINT# Demonstration
20 PRINT "PRINT# executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 52: Bad File Number, Error 54: Bad File Mode

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
| Name | PRINT# |
| Category | Filesystem I/O |
| Syntax | PRINT #file_num, expression_list |
| Description | Writes sequential formatted text data to an open disk file channel. |
| Error Summary | Error 2: Syntax Error, Error 52: Bad File Number, Error 54: Bad File Mode |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/file_ops/print_file.c |
