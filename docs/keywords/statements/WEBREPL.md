<!--
Title:        WEBREPL
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/stmt_webrepl.c
Generated:    no, hand-written
Status:       current
-->

# `WEBREPL` Keyword Reference

## Source Header

```c
// FILENAME: stmt_webrepl.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libserver (iot_net.h, iot_net.c)
// Implements WEBREPL.START and WEBREPL.STOP statements for over-the-air progr
//
// ---- Includes ----
```

## 1. Description & Usage

Starts wireless WebSocket WebREPL server for remote terminal interaction.

## 2. Syntax

```basic
WEBREPL.START [port] | WEBREPL.STOP
```

## 3. Code Example

```basic
10 REM WEBREPL Demonstration
20 PRINT "WEBREPL executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Wireless & IoT
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | WEBREPL |
| Category | Wireless & IoT |
| Syntax | WEBREPL.START [port] \| WEBREPL.STOP |
| Description | Starts wireless WebSocket WebREPL server for remote terminal interaction. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/stmt_webrepl.c |
