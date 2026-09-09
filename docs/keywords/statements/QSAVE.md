<!--
Title:        QSAVE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/binary_ops/stmt_qsave.c
Generated:    no, hand-written
Status:       current
-->

# `QSAVE` Keyword Reference

## Source Header

```c
// FILENAME: stmt_qsave.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libplatform
// Provides runtime implementation for the QSAVE statement in BASIC++.
```

## 1. Description & Usage

Saves binary package using modern 256-byte sector-aligned header with zero-malloc streaming.

## 2. Syntax

```basic
QSAVE filename$, offset [, length [, payload_type%]] [, {properties}]
```

## 3. Code Example

```basic
10 REM QSAVE Demonstration
20 PRINT "QSAVE executed successfully."
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
| Name | QSAVE |
| Category | File I/O & Memory |
| Syntax | QSAVE filename$, offset [, length [, payload_type%]] [, {properties}] |
| Description | Saves binary package using modern 256-byte sector-aligned header with zero-malloc streaming. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call, Error 70: Permission Denied |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/binary_ops/stmt_qsave.c |
