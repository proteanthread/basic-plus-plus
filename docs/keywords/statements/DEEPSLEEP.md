<!--
Title:        DEEPSLEEP
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/stmt_deepsleep.c
Generated:    no, hand-written
Status:       current
-->

# `DEEPSLEEP` Keyword Reference

## Source Header

```c
// FILENAME: stmt_deepsleep.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements DEEPSLEEP and LIGHTSLEEP power-saving statements.
//
// ---- Includes ----
```

## 1. Description & Usage

Enters ultra-low-power deep sleep standby mode for specified duration.

## 2. Syntax

```basic
DEEPSLEEP [ms] | LIGHTSLEEP [ms]
```

## 3. Code Example

```basic
10 REM DEEPSLEEP Demonstration
20 PRINT "DEEPSLEEP executed successfully."
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
| Name | DEEPSLEEP |
| Category | System & Power |
| Syntax | DEEPSLEEP [ms] \| LIGHTSLEEP [ms] |
| Description | Enters ultra-low-power deep sleep standby mode for specified duration. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/stmt_deepsleep.c |
