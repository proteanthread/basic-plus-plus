<!--
Title:        MESG
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/stmt_user_ipc.c
Generated:    no, hand-written
Status:       current
-->

# `MESG` Keyword Reference

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

Controls whether the current session user accepts incoming direct messages.

## 2. Syntax

```basic
MESG ON | OFF | Y | N
```

## 3. Code Example

```basic
10 REM MESG Demonstration
20 PRINT "MESG executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error

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
| Name | MESG |
| Category | System & Communications |
| Syntax | MESG ON \| OFF \| Y \| N |
| Description | Controls whether the current session user accepts incoming direct messages. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/stmt_user_ipc.c |
