<!--
Title:        SUSPEND
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/control/flow/suspend.c
Generated:    no, hand-written
Status:       current
-->

# `SUSPEND` Keyword Reference

## Source Header

```c
// FILENAME: suspend.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, language_descriptor.h)
// NEEDS: libcore (string.h)
// NEEDS: libengine (eval.h, eval.c, string.c, suspend.h, task.h, task.c, vm.h
// NEEDS: libkernel (errors.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the SUSPEND statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Suspends execution until the specified event occurs, key is pressed, timer triggers, or timeout expires.

## 2. Syntax

```basic
SUSPEND [TIMER | KEY | COM | TASK id | EVENT name$ | expr] [, timeout]
```

## 3. Code Example

```basic
10 REM SUSPEND Demonstration
20 PRINT "SUSPEND executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch, Error 70: Permission Denied

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
| Name | SUSPEND |
| Category | Control Flow |
| Syntax | SUSPEND [TIMER \| KEY \| COM \| TASK id \| EVENT name$ \| expr] [, timeout] |
| Description | Suspends execution until the specified event occurs, key is pressed, timer triggers, or timeout expires. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch, Error 70: Permission Denied |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/control/flow/suspend.c |
