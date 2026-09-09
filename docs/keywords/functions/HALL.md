<!--
Title:        HALL
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/func_hall.c
Generated:    no, hand-written
Status:       current
-->

# `HALL` Keyword Reference

## Source Header

```c
// FILENAME: func_hall.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, funcreg.c, iot_sensors.h, iot_sensors.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (vm.h)
// Implements the HALL.READ built-in function for magnetic Hall effect sensing
//
// ---- Includes ----
```

## 1. Description & Usage

Reads magnetic field intensity from ESP32 built-in Hall effect sensor.

## 2. Syntax

```basic
HALL() | HALL.READ()
```

## 3. Code Example

```basic
10 Val = HALL() | HALL.READ()
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

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
| Name | HALL |
| Category | Hardware & IoT |
| Syntax | HALL() \| HALL.READ() |
| Description | Reads magnetic field intensity from ESP32 built-in Hall effect sensor. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/func_hall.c |
