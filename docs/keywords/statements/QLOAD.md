<!--
Title:        QLOAD
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/binary_ops/stmt_qload.c
Generated:    no, hand-written
Status:       current
-->

# `QLOAD` Keyword Reference

## Source Header

```c
// FILENAME: stmt_qload.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libplatform
// Provides runtime implementation for the QLOAD statement in BASIC++.
```

## 1. Description & Usage

Loads binary memory package using modern 256-byte header or vintage 7-byte fallback with zero-malloc streaming.

## 2. Syntax

```basic
QLOAD filename$ [, address%]
```

## 3. Code Example

```basic
10 REM QLOAD Demonstration
20 PRINT "QLOAD executed successfully."
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
| Name | QLOAD |
| Category | File I/O & Memory |
| Syntax | QLOAD filename$ [, address%] |
| Description | Loads binary memory package using modern 256-byte header or vintage 7-byte fallback with zero-malloc streaming. |
| Error Summary | Error 2: Syntax Error, Error 53: File Not Found, Error 54: Bad File Mode |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/binary_ops/stmt_qload.c |
