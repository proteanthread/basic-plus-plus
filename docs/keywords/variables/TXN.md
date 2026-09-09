<!--
Title:        TXN
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/txn.c
Generated:    no, hand-written
Status:       current
-->

# `TXN` Keyword Reference

## Source Header

```c
// FILENAME: txn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h)
// NEEDS: libengine (eval.h, eval.c, stmt.h, string.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the TXN statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Provides ACID file transaction management and atomic block rollback capability.

## 2. Syntax

```basic
TXN BEGIN [FILE] | TXN COMMIT | TXN ROLLBACK | TXN STATUS | ATOMIC
```

## 3. Code Example

```basic
10 REM TXN Demonstration
20 PRINT "TXN executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 54: Bad File Mode, Error 57: Device I/O Error

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: File I/O
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | TXN |
| Category | File I/O |
| Syntax | TXN BEGIN [FILE] \| TXN COMMIT \| TXN ROLLBACK \| TXN STATUS \| ATOMIC |
| Description | Provides ACID file transaction management and atomic block rollback capability. |
| Error Summary | Error 2: Syntax Error, Error 54: Bad File Mode, Error 57: Device I/O Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/txn.c |
