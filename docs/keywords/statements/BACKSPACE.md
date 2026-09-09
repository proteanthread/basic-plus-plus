<!--
Title:        BACKSPACE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/binary_ops/backspace.c
Generated:    no, hand-written
Status:       current
-->

# `BACKSPACE` Keyword Reference

## Source Header

```c
// FILENAME: backspace.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h)
// NEEDS: libengine (backspace.h, eval.h, eval.c, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the BACKSPACE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Repositions the file pointer backwards by one record or block (IBM CALL/360 / DG).

## 2. Syntax

```basic
BACKSPACE [#]channel
```

## 3. Code Example

```basic
10 REM BACKSPACE Demonstration
20 PRINT "BACKSPACE executed successfully."
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
| Name | BACKSPACE |
| Category | File System & I/O |
| Syntax | BACKSPACE [#]channel |
| Description | Repositions the file pointer backwards by one record or block (IBM CALL/360 / DG). |
| Error Summary | Error 2: Syntax error, Error 13: Type mismatch, Error 52: Bad file number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/binary_ops/backspace.c |
