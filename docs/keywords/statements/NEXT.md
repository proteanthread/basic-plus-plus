<!--
Title:        NEXT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/loops/counter/next.c
Generated:    no, hand-written
Status:       current
-->

# `NEXT` Keyword Reference

## Source Header

```c
// FILENAME: next.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform
// Provides runtime implementation for the NEXT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Increments the FOR loop counter variable(s) and loops back if target bound has not been exceeded.

## 2. Syntax

```basic
NEXT [var1[, var2...]] | NEXT [var1 [var2...]]
```

## 3. Code Example

```basic
10 REM NEXT Demonstration
20 PRINT "NEXT executed successfully."
```

## 4. Error Conditions

Error 1: NEXT Without FOR, Error 2: Syntax Error

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Looping / Control Flow
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | NEXT |
| Category | Looping / Control Flow |
| Syntax | NEXT [var1[, var2...]] \| NEXT [var1 [var2...]] |
| Description | Increments the FOR loop counter variable(s) and loops back if target bound has not been exceeded. |
| Error Summary | Error 1: NEXT Without FOR, Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/loops/counter/next.c |
