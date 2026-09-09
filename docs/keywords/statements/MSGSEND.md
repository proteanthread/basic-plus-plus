<!--
Title:        MSGSEND
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/stmt_send.c
Generated:    no, hand-written
Status:       current
-->

# `MSGSEND` Keyword Reference

## Source Header

```c
// FILENAME: stmt_send.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_stmts.c
// NEEDS: libkernel (types/types.h, lexer/lexer.h, types/errors.h, msg_broker.
// Implementation for Point-to-Point Messaging Statements (SEND, MSGSEND).
//
// ---- Includes ----
```

## 1. Description & Usage

Alias for SEND. Dispatches a message to a named endpoint or user queue.

## 2. Syntax

```basic
MSGSEND target$, payload$
```

## 3. Code Example

```basic
10 REM MSGSEND Demonstration
20 PRINT "MSGSEND executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Communications & IPC
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | MSGSEND |
| Category | Communications & IPC |
| Syntax | MSGSEND target$, payload$ |
| Description | Alias for SEND. Dispatches a message to a named endpoint or user queue. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/stmt_send.c |
