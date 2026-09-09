<!--
Title:        INPUT#
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/file_ops/input_file.c
Generated:    no, hand-written
Status:       current
-->

# `INPUT#` Keyword Reference

## Source Header

```c
// FILENAME: input_file.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (line.c)
// NEEDS: libcore, libengine, libkernel
// Provides runtime implementation for the INPUT_FILE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Reads data items or full line text strings from an open sequential disk file channel or console.

## 2. Syntax

```basic
INPUT #file_num, var1 [, var2...] | LINE INPUT [#file_num,] string_var
```

## 3. Code Example

```basic
10 REM INPUT# Demonstration
20 PRINT "INPUT# executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 52: Bad File Number, Error 62: Input Past End

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
| Name | INPUT# |
| Category | Filesystem I/O |
| Syntax | INPUT #file_num, var1 [, var2...] \| LINE INPUT [#file_num,] string_var |
| Description | Reads data items or full line text strings from an open sequential disk file channel or console. |
| Error Summary | Error 2: Syntax Error, Error 52: Bad File Number, Error 62: Input Past End |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/file_ops/input_file.c |
