<!--
Title:        NEOPIXEL
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/stmt_neopixel.c
Generated:    no, hand-written
Status:       current
-->

# `NEOPIXEL` Keyword Reference

## Source Header

```c
// FILENAME: stmt_neopixel.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (iot_sensors.h, iot_sensors.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the NEOPIXEL statement for WS2812 addressable RGB LED control.
//
// ---- Includes ----
```

## 1. Description & Usage

Controls WS2812 / NeoPixel addressable RGB LED strip colors and latching.

## 2. Syntax

```basic
NEOPIXEL pin, index, r, g, b | NEOPIXEL.SHOW pin | NEOPIXEL.CLEAR pin
```

## 3. Code Example

```basic
10 REM NEOPIXEL Demonstration
20 PRINT "NEOPIXEL executed successfully."
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
| Name | NEOPIXEL |
| Category | Sensors & Actuators |
| Syntax | NEOPIXEL pin, index, r, g, b \| NEOPIXEL.SHOW pin \| NEOPIXEL.CLEAR pin |
| Description | Controls WS2812 / NeoPixel addressable RGB LED strip colors and latching. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/stmt_neopixel.c |
