<!--
Title:        QRUN
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/binary_ops/stmt_qrun.c
Generated:    no, hand-written
Status:       current
-->

# `QRUN` Keyword Reference

## Source Header

```c
// FILENAME: stmt_qrun.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libplatform
// Provides runtime implementation for the QRUN statement in BASIC++.
```

## 1. Description & Usage

Multi-modal super-runner for modern 256-byte binary packages, plugins, and vintage images with zero-malloc streaming.

## 2. Syntax

```basic
QRUN filename$ [, param$]
```

## 3. Code Example

```basic
10 REM QRUN Demonstration
20 PRINT "QRUN executed successfully."
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
| Name | QRUN |
| Category | File I/O & Execution |
| Syntax | QRUN filename$ [, param$] |
| Description | Multi-modal super-runner for modern 256-byte binary packages, plugins, and vintage images with zero-malloc streaming. |
| Error Summary | Error 2: Syntax Error, Error 53: File Not Found, Error 54: Bad File Mode |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/binary_ops/stmt_qrun.c |
