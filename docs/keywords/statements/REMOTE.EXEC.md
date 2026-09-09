<!--
Title:        REMOTE.EXEC
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/network/stmt_remote.c
Generated:    no, hand-written
Status:       current
-->

# `REMOTE.EXEC` Keyword Reference

## Source Header

```c
// FILENAME: stmt_remote.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (nil_transport.h, nil_transport.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, stmt_remote.h, string.c)
// Implements REMOTE.EXEC target$, cmd$ statement handler.
//
// ---- Includes ----
```

## 1. Description & Usage

Dispatches an execution command string to a remote IoT node.

## 2. Syntax

```basic
REMOTE.EXEC target$, cmd$
```

## 3. Code Example

```basic
10 REM REMOTE.EXEC Demonstration
20 PRINT "REMOTE.EXEC executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Hardware & Network
- **Subsystem**: SUBSYSTEM_SERVER
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | REMOTE.EXEC |
| Category | Hardware & Network |
| Syntax | REMOTE.EXEC target$, cmd$ |
| Description | Dispatches an execution command string to a remote IoT node. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_SERVER |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/network/stmt_remote.c |
