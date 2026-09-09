<!--
Title:        RAISE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/stmt_signal.c
Generated:    no, hand-written
Status:       current
-->

# `RAISE` Keyword Reference

## Source Header

```c
// FILENAME: stmt_signal.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_stmts.c
// NEEDS: libkernel (types/types.h, lexer/lexer.h, types/errors.h, msg_broker.
// Implementation for Signal Emission and Trapping Statements (RAISE SIGNAL, R
//
// ---- Includes ----
```

## 1. Description & Usage

Emits a synchronous or asynchronous signal event to active ON SIGNAL trap handlers and topic subscribers.

## 2. Syntax

```basic
RAISE [SIGNAL] topic$, payload$
```

## 3. Code Example

```basic
10 REM RAISE Demonstration
20 PRINT "RAISE executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Communications & Signaling
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | RAISE |
| Category | Communications & Signaling |
| Syntax | RAISE [SIGNAL] topic$, payload$ |
| Description | Emits a synchronous or asynchronous signal event to active ON SIGNAL trap handlers and topic subscribers. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/stmt_signal.c |
