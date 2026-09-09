<!--
Title:        I2C
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/stmt_i2c.c
Generated:    no, hand-written
Status:       current
-->

# `I2C` Keyword Reference

## Source Header

```c
// FILENAME: stmt_i2c.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strops.h, strops.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the I2C statement for Inter-Integrated Circuit bus read/write op
//
// ---- Includes ----
```

## 1. Description & Usage

Performs read or write transaction over I2C hardware bus.

## 2. Syntax

```basic
I2C.WRITE addr, reg, val | I2C.READ addr, reg, var
```

## 3. Code Example

```basic
10 REM I2C Demonstration
20 PRINT "I2C executed successfully."
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
| Name | I2C |
| Category | Hardware & IoT |
| Syntax | I2C.WRITE addr, reg, val \| I2C.READ addr, reg, var |
| Description | Performs read or write transaction over I2C hardware bus. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/stmt_i2c.c |
