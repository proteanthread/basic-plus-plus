<!--
Title:        BSAVE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/binary_ops/bsave.c
Generated:    no, hand-written
Status:       current
-->

# `BSAVE` Keyword Reference

## Source Header

```c
// FILENAME: bsave.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libplatform
// Provides runtime implementation for the BSAVE statement in BASIC++.
```

## 1. Description & Usage

Saves a block of memory with authentic 7-byte vintage binary header to disk.

## 2. Syntax

```basic
BSAVE filename$, offset%, length%
```

## 3. Code Example

```basic
10 REM BSAVE Demonstration
20 PRINT "BSAVE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call, Error 70: Permission Denied

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
| Name | BSAVE |
| Category | File I/O & Memory |
| Syntax | BSAVE filename$, offset%, length% |
| Description | Saves a block of memory with authentic 7-byte vintage binary header to disk. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call, Error 70: Permission Denied |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/binary_ops/bsave.c |
