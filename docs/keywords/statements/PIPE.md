<!--
Title:        PIPE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/file_ops/stmt_pipe.c
Generated:    no, hand-written
Status:       current
-->

# `PIPE` Keyword Reference

## Source Header

```c
// FILENAME: stmt_pipe.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h), libengine (eval.h, stmt.h), libkern
// Provides runtime implementation for PIPE and STREAMPIPE statements in BASIC
//
// ---- Includes ----
```

## 1. Description & Usage

Streams data continuously from source file/device channel to destination channel.

## 2. Syntax

```basic
PIPE #src_ch TO #dst_ch [BUFFER size] | STREAMPIPE #src, #dst
```

## 3. Code Example

```basic
10 REM PIPE Demonstration
20 PRINT "PIPE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 52: Bad File Number, Error 7: Out of Memory

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Filesystem & I/O
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | PIPE |
| Category | Filesystem & I/O |
| Syntax | PIPE #src_ch TO #dst_ch [BUFFER size] \| STREAMPIPE #src, #dst |
| Description | Streams data continuously from source file/device channel to destination channel. |
| Error Summary | Error 2: Syntax Error, Error 52: Bad File Number, Error 7: Out of Memory |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/file_ops/stmt_pipe.c |
