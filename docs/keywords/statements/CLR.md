<!--
Title:        CLR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/data/clr.c
Generated:    no, hand-written
Status:       current
-->

# `CLR` Keyword Reference

## Source Header

```c
// FILENAME: clr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (clr.h, lexer.h, lexer.c, string.c, vm.h)
// Provides runtime implementation for the CLR statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Clears specified variables or all variables to default zero/empty values.

## 2. Syntax

```basic
CLR [var1, var2, ...]
```

## 3. Code Example

```basic
10 REM CLR Demonstration
20 PRINT "CLR executed successfully."
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
| Name | CLR |
| Category | Variables & Memory |
| Syntax | CLR [var1, var2, ...] |
| Description | Clears specified variables or all variables to default zero/empty values. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/data/clr.c |
