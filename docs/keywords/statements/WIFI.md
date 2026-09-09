<!--
Title:        WIFI
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/stmt_wifi.c
Generated:    no, hand-written
Status:       current
-->

# `WIFI` Keyword Reference

## Source Header

```c
// FILENAME: stmt_wifi.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libserver (iot_net.h, iot_net.c)
// Implements WIFI.CONNECT and WIFI.DISCONNECT statements for wireless network
//
// ---- Includes ----
```

## 1. Description & Usage

Controls wireless 802.11 b/g/n station client, SoftAP hosting, scanning, and promiscuous packet sniffing.

## 2. Syntax

```basic
WIFI.CONNECT ssid$, pass$ | WIFI.DISCONNECT | WIFI.AP ssid$, pass$ [, ch, max] | WIFI.SCAN | WIFI.SNIFF ON|OFF [, ch]
```

## 3. Code Example

```basic
10 REM WIFI Demonstration
20 PRINT "WIFI executed successfully."
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
| Name | WIFI |
| Category | Wireless & IoT |
| Syntax | WIFI.CONNECT ssid$, pass$ \| WIFI.DISCONNECT \| WIFI.AP ssid$, pass$ [, ch, max] \| WIFI.SCAN \| WIFI.SNIFF ON\|OFF [, ch] |
| Description | Controls wireless 802.11 b/g/n station client, SoftAP hosting, scanning, and promiscuous packet sniffing. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/stmt_wifi.c |
