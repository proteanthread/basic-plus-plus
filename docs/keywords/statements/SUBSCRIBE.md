<!--
Title:        SUBSCRIBE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/stmt_subscribe.c
Generated:    no, hand-written
Status:       current
-->

# `SUBSCRIBE` Keyword Reference

## Source Header

```c
// FILENAME: stmt_subscribe.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_stmts.c
// NEEDS: libkernel (types/types.h, lexer/lexer.h, types/errors.h, msg_broker.
// Implementation for Topic Subscription Statement (SUBSCRIBE).
//
// ---- Includes ----
```

## 1. Description & Usage

Subscribes the active session or task context to receive incoming messages on the specified topic.

## 2. Syntax

```basic
SUBSCRIBE topic$
```

## 3. Code Example

```basic
10 REM SUBSCRIBE Demonstration
20 PRINT "SUBSCRIBE executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Communications & PubSub
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | SUBSCRIBE |
| Category | Communications & PubSub |
| Syntax | SUBSCRIBE topic$ |
| Description | Subscribes the active session or task context to receive incoming messages on the specified topic. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/stmt_subscribe.c |
