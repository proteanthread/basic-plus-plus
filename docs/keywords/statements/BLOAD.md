<!--
Title:        BLOAD
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/binary_ops/bload.c
Generated:    no, hand-written
Status:       current
-->

# `BLOAD` Keyword Reference

## Source Header

```c
// FILENAME: bload.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libplatform
// Provides runtime implementation for the BLOAD statement in BASIC++.
```

## 1. Description & Usage

Loads a memory image file created by BSAVE with 7-byte header verification.

## 2. Syntax

```basic
BLOAD filename$ [, offset%]
```

## 3. Code Example

```basic
10 REM BLOAD Demonstration
20 PRINT "BLOAD executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 53: File Not Found, Error 54: Bad File Mode

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: File I/O & Memory
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | BLOAD |
| Category | File I/O & Memory |
| Syntax | BLOAD filename$ [, offset%] |
| Description | Loads a memory image file created by BSAVE with 7-byte header verification. |
| Error Summary | Error 2: Syntax Error, Error 53: File Not Found, Error 54: Bad File Mode |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/binary_ops/bload.c |
