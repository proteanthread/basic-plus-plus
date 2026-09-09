<!--
Title:        ESPNOW
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/wireless/stmt_espnow.c
Generated:    no, hand-written
Status:       current
-->

# `ESPNOW` Keyword Reference

## Source Header

```c
// FILENAME: stmt_espnow.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libserver
// Implements the ESPNOW statement for fast peer-to-peer radio linking.
//
// ---- Includes ----
```

## 1. Description & Usage

Transmits connectionless low-latency 2.4 GHz packets between ESP32 peers without Wi-Fi router.

## 2. Syntax

```basic
ESPNOW.INIT [channel] | ESPNOW.ADD.PEER mac$ [, channel] | ESPNOW.SEND mac$, data$ | ESPNOW.RECV var$
```

## 3. Code Example

```basic
10 REM ESPNOW Demonstration
20 PRINT "ESPNOW executed successfully."
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
| Name | ESPNOW |
| Category | Wireless & IoT |
| Syntax | ESPNOW.INIT [channel] \| ESPNOW.ADD.PEER mac$ [, channel] \| ESPNOW.SEND mac$, data$ \| ESPNOW.RECV var$ |
| Description | Transmits connectionless low-latency 2.4 GHz packets between ESP32 peers without Wi-Fi router. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/wireless/stmt_espnow.c |
