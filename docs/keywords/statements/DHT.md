<!--
Title:        DHT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/stmt_dht.c
Generated:    no, hand-written
Status:       current
-->

# `DHT` Keyword Reference

## Source Header

```c
// FILENAME: stmt_dht.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (iot_sensors.h, iot_sensors.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strops.h, strops.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the DHT.READ statement for digital temperature and humidity sens
//
// ---- Includes ----
```

## 1. Description & Usage

Reads temperature (Celsius) and relative humidity (%) from a DHT11 or DHT22 sensor.

## 2. Syntax

```basic
DHT.READ pin, temp_var, hum_var
```

## 3. Code Example

```basic
10 REM DHT Demonstration
20 PRINT "DHT executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Sensors & Actuators
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DHT |
| Category | Sensors & Actuators |
| Syntax | DHT.READ pin, temp_var, hum_var |
| Description | Reads temperature (Celsius) and relative humidity (%) from a DHT11 or DHT22 sensor. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/stmt_dht.c |
