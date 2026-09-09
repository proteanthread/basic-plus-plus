<!--
Title:        DREAD
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/func_dread.c
Generated:    no, hand-written
Status:       current
-->

# `DREAD` Keyword Reference

## Source Header

```c
// FILENAME: func_dread.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c, funcreg.h, funcreg.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (vm.h)
// Implements the DREAD built-in function to read digital pin states.
//
// ---- Includes ----
```

## 1. Description & Usage

Reads digital state (0 or 1) from specified microcontroller pin.

## 2. Syntax

```basic
DREAD(pin)
```

## 3. Code Example

```basic
10 Val = DREAD(pin)
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
| Name | DREAD |
| Category | Hardware & IoT |
| Syntax | DREAD(pin) |
| Description | Reads digital state (0 or 1) from specified microcontroller pin. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/func_dread.c |
