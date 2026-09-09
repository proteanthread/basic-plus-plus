<!--
Title:        DAC
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/stmt_dac.c
Generated:    no, hand-written
Status:       current
-->

# `DAC` Keyword Reference

## Source Header

```c
// FILENAME: stmt_dac.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the DAC.WRITE statement for digital-to-analog converter voltage 
//
// ---- Includes ----
```

## 1. Description & Usage

Outputs an 8-bit analog voltage (0-255) on a hardware DAC pin (e.g. GPIO 25/26).

## 2. Syntax

```basic
DAC pin, value | DAC.WRITE pin, value
```

## 3. Code Example

```basic
10 REM DAC Demonstration
20 PRINT "DAC executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

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
| Name | DAC |
| Category | Hardware & IoT |
| Syntax | DAC pin, value \| DAC.WRITE pin, value |
| Description | Outputs an 8-bit analog voltage (0-255) on a hardware DAC pin (e.g. GPIO 25/26). |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/stmt_dac.c |
