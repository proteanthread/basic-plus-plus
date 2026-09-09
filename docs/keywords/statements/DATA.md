<!--
Title:        DATA
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/data/data.c
Generated:    no, hand-written
Status:       current
-->

# `DATA` Keyword Reference

## Source Header

```c
// FILENAME: data.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (data.h, lexer.h, lexer.c, string.c, vm.h)
// Provides runtime implementation for the DATA statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Stores static numeric and string constants to be read sequentially into variables.

## 2. Syntax

```basic
DATA constant1 [, constant2...]
```

## 3. Code Example

```basic
10 REM DATA Demonstration
20 PRINT "DATA executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error

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
| Name | DATA |
| Category | Variables & Memory |
| Syntax | DATA constant1 [, constant2...] |
| Description | Stores static numeric and string constants to be read sequentially into variables. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/data/data.c |
