<!--
Title:        VOID
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/control/flow/void.c
Generated:    no, hand-written
Status:       current
-->

# `VOID` Keyword Reference

## Source Header

```c
// FILENAME: void.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, map.h, map.c, vm.h)
// NEEDS: libengine (void.h)
// Provides runtime implementation for the VOID statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Evaluates an expression or function for side-effects and discards the return value.

## 2. Syntax

```basic
VOID expression
```

## 3. Code Example

```basic
10 REM VOID Demonstration
20 PRINT "VOID executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Control Flow
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | VOID |
| Category | Control Flow |
| Syntax | VOID expression |
| Description | Evaluates an expression or function for side-effects and discards the return value. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/control/flow/void.c |
