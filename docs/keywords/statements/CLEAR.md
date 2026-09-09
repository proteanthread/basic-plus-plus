<!--
Title:        CLEAR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/program/clear.c
Generated:    no, hand-written
Status:       current
-->

# `CLEAR` Keyword Reference

## Source Header

```c
// FILENAME: clear.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (clear.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the CLEAR statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Frees variable and array memory, closes open files, and sets optional memory limits.

## 2. Syntax

```basic
CLEAR [string_space%] | CLEAR [, [mem_limit%] [, stack_size%]]
```

## 3. Code Example

```basic
10 REM CLEAR Demonstration
20 PRINT "CLEAR executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

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
| Name | CLEAR |
| Category | Variables & Memory |
| Syntax | CLEAR [string_space%] \| CLEAR [, [mem_limit%] [, stack_size%]] |
| Description | Frees variable and array memory, closes open files, and sets optional memory limits. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/program/clear.c |
