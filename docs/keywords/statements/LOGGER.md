<!--
Title:        LOGGER
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/stmt_user_ipc.c
Generated:    no, hand-written
Status:       current
-->

# `LOGGER` Keyword Reference

## Source Header

```c
// FILENAME: stmt_user_ipc.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c
// NEEDS: libkernel (types.h, lexer.h, errors.h, vdev.h, dev_user.h), libcore 
// Implementation for Inter-User Messaging statements (WALL, MESG, WRITE, TALK
//
// ---- Includes ----
```

## 1. Description & Usage

Writes a timestamped system event runtime_log entry to USER0: / syslog sink.

## 2. Syntax

```basic
LOGGER message$ [, level]
```

## 3. Code Example

```basic
10 REM LOGGER Demonstration
20 PRINT "LOGGER executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Communications
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | LOGGER |
| Category | System & Communications |
| Syntax | LOGGER message$ [, level] |
| Description | Writes a timestamped system event runtime_log entry to USER0: / syslog sink. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/stmt_user_ipc.c |
