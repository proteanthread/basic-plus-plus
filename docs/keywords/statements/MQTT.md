<!--
Title:        MQTT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/stmt_mqtt.c
Generated:    no, hand-written
Status:       current
-->

# `MQTT` Keyword Reference

## Source Header

```c
// FILENAME: stmt_mqtt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libserver (iot_net.h, iot_net.c)
// Implements MQTT.PUBLISH and MQTT.SUBSCRIBE statements for IoT cloud messagi
//
// ---- Includes ----
```

## 1. Description & Usage

Publishes telemetry or subscribes to message topics via MQTT broker.

## 2. Syntax

```basic
MQTT.PUBLISH topic$, payload$ | MQTT.SUBSCRIBE topic$
```

## 3. Code Example

```basic
10 REM MQTT Demonstration
20 PRINT "MQTT executed successfully."
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
| Name | MQTT |
| Category | Wireless & IoT |
| Syntax | MQTT.PUBLISH topic$, payload$ \| MQTT.SUBSCRIBE topic$ |
| Description | Publishes telemetry or subscribes to message topics via MQTT broker. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/stmt_mqtt.c |
