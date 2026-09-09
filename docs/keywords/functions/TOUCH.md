<!--
Title:        TOUCH
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/func_touch.c
Generated:    no, hand-written
Status:       current
-->

# `TOUCH` Keyword Reference

## Source Header

```c
// FILENAME: func_touch.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, funcreg.c, iot_sensors.h, iot_sensors.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (vm.h)
// Implements the TOUCH.READ built-in function for capacitive touch pin sensin
//
// ---- Includes ----
```

## 1. Description & Usage

Reads capacitive touch sensor threshold from microcontroller pin.

## 2. Syntax

```basic
TOUCH(pin) | TOUCH.READ(pin)
```

## 3. Code Example

```basic
10 Val = TOUCH(pin) | TOUCH.READ(pin)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Hardware & IoT
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | TOUCH |
| Category | Hardware & IoT |
| Syntax | TOUCH(pin) \| TOUCH.READ(pin) |
| Description | Reads capacitive touch sensor threshold from microcontroller pin. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/func_touch.c |
