<!--
Title:        REWIND
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/binary_ops/rewind.c
Generated:    no, hand-written
Status:       current
-->

# `REWIND` Keyword Reference

## Source Header

```c
// FILENAME: rewind.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h)
// NEEDS: libengine (eval.h, eval.c, rewind.h, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the REWIND statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Rewinds the read/write pointer to the beginning of the specified file channel (HP 2000 TSB).

## 2. Syntax

```basic
REWIND [#]channel
```

## 3. Code Example

```basic
10 REM REWIND Demonstration
20 PRINT "REWIND executed successfully."
```

## 4. Error Conditions

Error 2: Syntax error, Error 13: Type mismatch, Error 52: Bad file number

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: File System & I/O
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | REWIND |
| Category | File System & I/O |
| Syntax | REWIND [#]channel |
| Description | Rewinds the read/write pointer to the beginning of the specified file channel (HP 2000 TSB). |
| Error Summary | Error 2: Syntax error, Error 13: Type mismatch, Error 52: Bad file number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/binary_ops/rewind.c |
