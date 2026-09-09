<!--
Title:        BAUD
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/stmt_baud.c
Generated:    no, hand-written
Status:       current
-->

# `BAUD` Keyword Reference

## Source Header

```c
// FILENAME: stmt_baud.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, stmt_baud.h, func_baud.h, vm.h)
// Provides runtime implementation for the BAUD statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Configures simulated transmission speed in bps (from vintage Baudot 45.45 bps up to 115200+ bps) on channel (0 for console).

## 2. Syntax

```basic
BAUD rate# | BAUD = rate# | BAUD [#]channel%, rate#
```

## 3. Code Example

```basic
10 REM BAUD Demonstration
20 PRINT "BAUD executed successfully."
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
| Name | BAUD |
| Category | System & Hardware |
| Syntax | BAUD rate# \| BAUD = rate# \| BAUD [#]channel%, rate# |
| Description | Configures simulated transmission speed in bps (from vintage Baudot 45.45 bps up to 115200+ bps) on channel (0 for console). |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/stmt_baud.c |
