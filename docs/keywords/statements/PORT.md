<!--
Title:        PORT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/network/stmt_port_trigger.c
Generated:    no, hand-written
Status:       current
-->

# `PORT` Keyword Reference

## Source Header

```c
// FILENAME: stmt_port_trigger.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, events_net.h, events_net.c)
// NEEDS: libengine (stmt_port_trigger.h)
// Implements PORT statement for port knocking, triggering and trapping.
//
// ---- Includes ----
```

## 1. Description & Usage

Controls software port event trapping, firing, and triggering.

## 2. Syntax

```basic
PORT.FIRE port_num | PORT(p) ON|OFF|STOP
```

## 3. Code Example

```basic
10 Val = PORT.FIRE port_num | PORT(p) ON|OFF|STOP
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Syntax error in PORT statement

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
| Name | PORT |
| Category | Hardware & Network |
| Syntax | PORT.FIRE port_num \| PORT(p) ON\|OFF\|STOP |
| Description | Controls software port event trapping, firing, and triggering. |
| Error Summary | Error 5: Syntax error in PORT statement |
| Subsystem | SUBSYSTEM_SERVER |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/network/stmt_port_trigger.c |
