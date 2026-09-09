<!--
Title:        WRITE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/io/file_ops/write.c
Generated:    no, hand-written
Status:       current
-->

# `WRITE` Keyword Reference

## Source Header

```c
// FILENAME: write.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for WRITE.
```

## 1. Description & Usage

Outputs comma-delimited, quote-encapsulated data values suitable for subsequent INPUT reading.

## 2. Syntax

```basic
WRITE [#file_num,] [exprlist]
```

## 3. Code Example

```basic
10 REM WRITE Demonstration
20 PRINT "WRITE executed successfully."
```

## 4. Error Conditions

Error 52: Bad File Number

## 5. Compatibility & Lineage

- **Lineage**: GW-BASIC, QBASIC, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: File & Console I/O
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | WRITE |
| Category | File & Console I/O |
| Syntax | WRITE [#file_num,] [exprlist] |
| Description | Outputs comma-delimited, quote-encapsulated data values suitable for subsequent INPUT reading. |
| Error Summary | Error 52: Bad File Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | GW-BASIC, QBASIC, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/statements/io/file_ops/write.c |
