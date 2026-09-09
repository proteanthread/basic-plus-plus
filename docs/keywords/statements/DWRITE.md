<!--
Title:        DWRITE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/stmt_dwrite.c
Generated:    no, hand-written
Status:       current
-->

# `DWRITE` Keyword Reference

## Source Header

```c
// FILENAME: stmt_dwrite.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the DWRITE statement for digital pin output control.
//
// ---- Includes ----
```

## 1. Description & Usage

Writes a digital HIGH (1) or LOW (0) value to a microcontroller pin.

## 2. Syntax

```basic
DWRITE pin, value
```

## 3. Code Example

```basic
10 REM DWRITE Demonstration
20 PRINT "DWRITE executed successfully."
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
| Name | DWRITE |
| Category | Hardware & IoT |
| Syntax | DWRITE pin, value |
| Description | Writes a digital HIGH (1) or LOW (0) value to a microcontroller pin. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/stmt_dwrite.c |
