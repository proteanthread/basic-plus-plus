<!--
Title:        SHARED
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/oop/shared.c
Generated:    no, hand-written
Status:       current
-->

# `SHARED` Keyword Reference

## Source Header

```c
// FILENAME: shared.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (share.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, shared.h, string.c, vm.
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the SHARED statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Grants SUB or FUNCTION procedures access to module-level global variables.

## 2. Syntax

```basic
SHARED variable [, variable...]
```

## 3. Code Example

```basic
10 REM SHARED Demonstration
20 PRINT "SHARED executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 33: Illegal Outside SUB/FUNCTION

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
| Name | SHARED |
| Category | Variables & Memory |
| Syntax | SHARED variable [, variable...] |
| Description | Grants SUB or FUNCTION procedures access to module-level global variables. |
| Error Summary | Error 2: Syntax Error, Error 33: Illegal Outside SUB/FUNCTION |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/oop/shared.c |
