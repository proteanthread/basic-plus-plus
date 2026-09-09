<!--
Title:        AREAD
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/func_aread.c
Generated:    no, hand-written
Status:       current
-->

# `AREAD` Keyword Reference

## Source Header

```c
// FILENAME: func_aread.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c, funcreg.h, funcreg.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (vm.h)
// Implements the AREAD built-in function to read analog pin values.
//
// ---- Includes ----
```

## 1. Description & Usage

Reads analog value (0-4095) from specified microcontroller ADC pin.

## 2. Syntax

```basic
AREAD(pin)
```

## 3. Code Example

```basic
10 Val = AREAD(pin)
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
| Name | AREAD |
| Category | Hardware & IoT |
| Syntax | AREAD(pin) |
| Description | Reads analog value (0-4095) from specified microcontroller ADC pin. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/func_aread.c |
