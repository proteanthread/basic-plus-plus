<!--
Title:        LOG
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/stmt_log.c
Generated:    no, hand-written
Status:       current
-->

# `LOG` Keyword Reference

## Source Header

```c
// FILENAME: stmt_log.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_stmts.c
// NEEDS: libkernel (types/types.h, lexer/lexer.h, types/errors.h, vdev.h), li
// Implementation for Structured and Flat Logging Statements (LOG.INFO, LOG.WA
//
// ---- Includes ----
```

## 1. Description & Usage

Emits structured runtime_log messages across multiple severity levels or manages the circular runtime_log ring buffer.

## 2. Syntax

```basic
LOG.INFO msg$ [, tag$] | LOG LEVEL level$ | LOG DUMP [level$] | LOG CLEAR
```

## 3. Code Example

```basic
10 REM LOG Demonstration
20 PRINT "LOG executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Logging
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | LOG |
| Category | System & Logging |
| Syntax | LOG.INFO msg$ [, tag$] \| LOG LEVEL level$ \| LOG DUMP [level$] \| LOG CLEAR |
| Description | Emits structured runtime_log messages across multiple severity levels or manages the circular runtime_log ring buffer. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/stmt_log.c |
