<!--
Title:        EXEC$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/environment/func_exec.c
Generated:    no, hand-written
Status:       current
-->

# `EXEC$` Keyword Reference

## Source Header

```c
// FILENAME: func_exec.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libengine (func_exec.h, vm.h)
// Provides runtime implementation for the EXEC$ direct process execution func
```

## 1. Description & Usage

Directly executes a binary executable with arguments and captures standard output without shell interpretation.

## 2. Syntax

```basic
EXEC$(binary$ [, arg1$ [, arg2$...]])
```

## 3. Code Example

```basic
10 Val = EXEC$(binary$ [, arg1$ [, arg2$...]])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & OS
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | EXEC$ |
| Category | System & OS |
| Syntax | EXEC$(binary$ [, arg1$ [, arg2$...]]) |
| Description | Directly executes a binary executable with arguments and captures standard output without shell interpretation. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/environment/func_exec.c |
