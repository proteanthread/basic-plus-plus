<!--
Title:        BT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/wireless/stmt_bluetooth.c
Generated:    no, hand-written
Status:       current
-->

# `BT` Keyword Reference

## Source Header

```c
// FILENAME: stmt_bluetooth.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strops.h, strops.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, stmt_bluetooth.h)
// NEEDS: libengine (string.c, vm.h)
// NEEDS: libserver (iot_net.h, iot_net.c)
// Implements BT and BLE statements for Bluetooth Classic and BLE operations.
//
// ---- Includes ----
```

## 1. Description & Usage

Controls Bluetooth Classic Serial Port Profile (SPP) virtual COM link.

## 2. Syntax

```basic
BT.START name$ | BT.CONNECT mac_or_name$
```

## 3. Code Example

```basic
10 REM BT Demonstration
20 PRINT "BT executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

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
| Name | BT |
| Category | Wireless & IoT |
| Syntax | BT.START name$ \| BT.CONNECT mac_or_name$ |
| Description | Controls Bluetooth Classic Serial Port Profile (SPP) virtual COM link. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/wireless/stmt_bluetooth.c |
