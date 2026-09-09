<!--
Title:        BRUN
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/binary_ops/brun.c
Generated:    no, hand-written
Status:       current
-->

# `BRUN` Keyword Reference

## Source Header

```c
// FILENAME: brun.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libplatform
// Provides runtime implementation for the BRUN statement in BASIC++.
```

## 1. Description & Usage

Loads a vintage 7-byte binary memory image into memory and executes it.

## 2. Syntax

```basic
BRUN filename$ [, address%]
```

## 3. Code Example

```basic
10 REM BRUN Demonstration
20 PRINT "BRUN executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 53: File Not Found, Error 54: Bad File Mode

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: File I/O & Execution
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | BRUN |
| Category | File I/O & Execution |
| Syntax | BRUN filename$ [, address%] |
| Description | Loads a vintage 7-byte binary memory image into memory and executes it. |
| Error Summary | Error 2: Syntax Error, Error 53: File Not Found, Error 54: Bad File Mode |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/binary_ops/brun.c |
