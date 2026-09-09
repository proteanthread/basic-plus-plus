<!--
Title:        MSGRECV$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/ipc/func_receive.c
Generated:    no, hand-written
Status:       current
-->

# `MSGRECV$` Keyword Reference

## Source Header

```c
// FILENAME: func_receive.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_funcs.c
// NEEDS: libkernel (types.h, errors.h, msg_broker.h, dev_user.h), libcore (st
// Implementation for Point-to-Point Messaging Functions (RECEIVE$, MSGRECV$).
//
// ---- Includes ----
```

## 1. Description & Usage

Alias for RECEIVE$. Pulls next message from a named endpoint or user queue.

## 2. Syntax

```basic
MSGRECV$(source$ [, timeout_ms])
```

## 3. Code Example

```basic
10 Val = MSGRECV$(source$ [, timeout_ms])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call, Error 13: Type Mismatch

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
| Name | MSGRECV$ |
| Category | Communications & IPC |
| Syntax | MSGRECV$(source$ [, timeout_ms]) |
| Description | Alias for RECEIVE$. Pulls next message from a named endpoint or user queue. |
| Error Summary | Error 5: Illegal Function Call, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/ipc/func_receive.c |
