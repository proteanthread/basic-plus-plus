<!--
Title:        SEND
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/stmt_send.c
Generated:    no, hand-written
Status:       current
-->

# `SEND` Keyword Reference

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

Sends a point-to-point message to the specified endpoint, channel, or user mailbox queue.

## 2. Syntax

```basic
SEND target$, payload$
```

## 3. Code Example

```basic
10 REM SEND Demonstration
20 PRINT "SEND executed successfully."
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
| Name | SEND |
| Category | Communications & IPC |
| Syntax | SEND target$, payload$ |
| Description | Sends a point-to-point message to the specified endpoint, channel, or user mailbox queue. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/stmt_send.c |
