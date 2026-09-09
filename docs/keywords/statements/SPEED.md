<!--
Title:        SPEED
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/stmt_speed.c
Generated:    no, hand-written
Status:       current
-->

# `SPEED` Keyword Reference

## Source Header

```c
// FILENAME: stmt_speed.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, stmt_speed.h, func_baud.h, vm.h)
// Provides runtime implementation for the SPEED statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Configures console output throttling speed. Values <= 255 set Apple II delay scale (0..255); values > 255 or SPEED& set baud rate in bps (45.45 up to 115200+).

## 2. Syntax

```basic
SPEED [=] rate | SPEED% = val% | SPEED& = rate&
```

## 3. Code Example

```basic
10 REM SPEED Demonstration
20 PRINT "SPEED executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Hardware
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | SPEED |
| Category | System & Hardware |
| Syntax | SPEED [=] rate \| SPEED% = val% \| SPEED& = rate& |
| Description | Configures console output throttling speed. Values <= 255 set Apple II delay scale (0..255); values > 255 or SPEED& set baud rate in bps (45.45 up to 115200+). |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/stmt_speed.c |
