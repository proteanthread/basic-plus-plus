<!--
Title:        FREQ
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/stmt_freq.c
Generated:    no, hand-written
Status:       current
-->

# `FREQ` Keyword Reference

## Source Header

```c
// FILENAME: stmt_freq.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the FREQ statement for microcontroller CPU frequency configurati
//
// ---- Includes ----
```

## 1. Description & Usage

Sets CPU clock frequency (e.g. 80, 160, 240 MHz on ESP32).

## 2. Syntax

```basic
FREQ mhz
```

## 3. Code Example

```basic
10 REM FREQ Demonstration
20 PRINT "FREQ executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Power
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | FREQ |
| Category | System & Power |
| Syntax | FREQ mhz |
| Description | Sets CPU clock frequency (e.g. 80, 160, 240 MHz on ESP32). |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/stmt_freq.c |
