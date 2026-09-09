<!--
Title:        DELAY
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/stmt_delay.c
Generated:    no, hand-written
Status:       current
-->

# `DELAY` Keyword Reference

## Source Header

```c
// FILENAME: stmt_delay.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the DELAY statement for millisecond and microsecond sleep pauses
//
// ---- Includes ----
```

## 1. Description & Usage

Pauses execution for specified milliseconds or microseconds.

## 2. Syntax

```basic
DELAY ms | DELAY.MS ms | DELAY.US us
```

## 3. Code Example

```basic
10 REM DELAY Demonstration
20 PRINT "DELAY executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Timing & Real-Time
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DELAY |
| Category | Timing & Real-Time |
| Syntax | DELAY ms \| DELAY.MS ms \| DELAY.US us |
| Description | Pauses execution for specified milliseconds or microseconds. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/stmt_delay.c |
