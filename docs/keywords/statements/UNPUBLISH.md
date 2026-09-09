<!--
Title:        UNPUBLISH
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/stmt_unpublish.c
Generated:    no, hand-written
Status:       current
-->

# `UNPUBLISH` Keyword Reference

## Source Header

```c
// FILENAME: stmt_unpublish.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_stmts.c
// NEEDS: libkernel (types/types.h, lexer/lexer.h, types/errors.h, msg_broker.
// Implementation for Topic Unpublish Statement (UNPUBLISH).
//
// ---- Includes ----
```

## 1. Description & Usage

Unpublishes a topic, withdraws a queued message payload, or resets all active pubsub topics.

## 2. Syntax

```basic
UNPUBLISH topic$ [, payload$] | UNPUBLISH ALL
```

## 3. Code Example

```basic
10 REM UNPUBLISH Demonstration
20 PRINT "UNPUBLISH executed successfully."
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
| Name | UNPUBLISH |
| Category | Communications & PubSub |
| Syntax | UNPUBLISH topic$ [, payload$] \| UNPUBLISH ALL |
| Description | Unpublishes a topic, withdraws a queued message payload, or resets all active pubsub topics. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/stmt_unpublish.c |
