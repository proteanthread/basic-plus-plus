<!--
Title:        LOGDEBUG
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/stmt_log.c
Generated:    no, hand-written
Status:       current
-->

# `LOGDEBUG` Keyword Reference

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

Emits a DEBUG-severity runtime_log entry to the active logging sinks and circular ring buffer.

## 2. Syntax

```basic
LOGDEBUG message$ [, tag$]
```

## 3. Code Example

```basic
10 REM LOGDEBUG Demonstration
20 PRINT "LOGDEBUG executed successfully."
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
| Name | LOGDEBUG |
| Category | System & Logging |
| Syntax | LOGDEBUG message$ [, tag$] |
| Description | Emits a DEBUG-severity runtime_log entry to the active logging sinks and circular ring buffer. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/stmt_log.c |
