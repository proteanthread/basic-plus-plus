<!--
Title:        SERVO
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/stmt_servo.c
Generated:    no, hand-written
Status:       current
-->

# `SERVO` Keyword Reference

## Source Header

```c
// FILENAME: stmt_servo.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the SERVO statement for hobby servo motor angular positioning.
//
// ---- Includes ----
```

## 1. Description & Usage

Positions a servo motor on the specified pin to an angle between 0 and 180 degrees.

## 2. Syntax

```basic
SERVO pin, angle
```

## 3. Code Example

```basic
10 REM SERVO Demonstration
20 PRINT "SERVO executed successfully."
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
| Name | SERVO |
| Category | Hardware & IoT |
| Syntax | SERVO pin, angle |
| Description | Positions a servo motor on the specified pin to an angle between 0 and 180 degrees. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/stmt_servo.c |
