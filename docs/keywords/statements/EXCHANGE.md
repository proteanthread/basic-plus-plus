<!--
Title:        EXCHANGE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/assignment/exchange.c
Generated:    no, hand-written
Status:       current
-->

# `EXCHANGE` Keyword Reference

## Source Header

```c
// FILENAME: exchange.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (exchange.h, string.c, swap.h, swap.c)
// Provides runtime implementation for the EXCHANGE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Apple /// Business BASIC alias for SWAP. Exchanges values between two variables or array elements.

## 2. Syntax

```basic
EXCHANGE var1, var2
```

## 3. Code Example

```basic
10 REM EXCHANGE Demonstration
20 PRINT "EXCHANGE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Variables & Memory
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | EXCHANGE |
| Category | Variables & Memory |
| Syntax | EXCHANGE var1, var2 |
| Description | Apple /// Business BASIC alias for SWAP. Exchanges values between two variables or array elements. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/assignment/exchange.c |
