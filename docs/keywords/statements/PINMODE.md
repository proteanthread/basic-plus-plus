<!--
Title:        PINMODE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/stmt_pinmode.c
Generated:    no, hand-written
Status:       current
-->

# `PINMODE` Keyword Reference

## Source Header

```c
// FILENAME: stmt_pinmode.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the PINMODE statement for microcontroller pin configuration.
//
// ---- Includes ----
```

## 1. Description & Usage

Configures a microcontroller GPIO pin mode (INPUT, OUTPUT, PULLUP, PULLDOWN).

## 2. Syntax

```basic
PINMODE pin, mode
```

## 3. Code Example

```basic
10 REM PINMODE Demonstration
20 PRINT "PINMODE executed successfully."
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
| Name | PINMODE |
| Category | Hardware & IoT |
| Syntax | PINMODE pin, mode |
| Description | Configures a microcontroller GPIO pin mode (INPUT, OUTPUT, PULLUP, PULLDOWN). |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/stmt_pinmode.c |
