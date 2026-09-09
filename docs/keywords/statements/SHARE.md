<!--
Title:        SHARE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/declaration/share.c
Generated:    no, hand-written
Status:       current
-->

# `SHARE` Keyword Reference

## Source Header

```c
// FILENAME: share.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (lexer.h, lexer.c, share.h, shared.h, shared.c, string.c)
// NEEDS: libengine (vm.h)
// Provides runtime implementation for the SHARE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

ECMA-116 standard statement to share variables between module routines and subprograms.

## 2. Syntax

```basic
SHARE variable [, variable...]
```

## 3. Code Example

```basic
10 REM SHARE Demonstration
20 PRINT "SHARE executed successfully."
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
| Name | SHARE |
| Category | Variables & Memory |
| Syntax | SHARE variable [, variable...] |
| Description | ECMA-116 standard statement to share variables between module routines and subprograms. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/declaration/share.c |
