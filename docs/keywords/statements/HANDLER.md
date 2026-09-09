<!--
Title:        HANDLER
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/control/branch/handler.c
Generated:    no, hand-written
Status:       current
-->

# `HANDLER` Keyword Reference

## Source Header

```c
// FILENAME: handler.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (handler.h, lexer.h, lexer.c, string.c, vm.h)
// Provides runtime implementation for the HANDLER statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

ECMA-116 standard detached exception handler definition.

## 2. Syntax

```basic
HANDLER name$ ... END HANDLER
```

## 3. Code Example

```basic
10 REM HANDLER Demonstration
20 PRINT "HANDLER executed successfully."
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
| Name | HANDLER |
| Category | Control Flow |
| Syntax | HANDLER name$ ... END HANDLER |
| Description | ECMA-116 standard detached exception handler definition. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/control/branch/handler.c |
