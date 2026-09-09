<!--
Title:        SLEEP
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/control/flow/sleep.c
Generated:    no, hand-written
Status:       current
-->

# `SLEEP` Keyword Reference

## Source Header

```c
// FILENAME: sleep.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, sleep.h, string.c)
// NEEDS: libkernel (errors.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the SLEEP statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Suspends program execution for the specified duration in seconds.

## 2. Syntax

```basic
SLEEP [seconds]
```

## 3. Code Example

```basic
10 REM SLEEP Demonstration
20 PRINT "SLEEP executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

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
| Name | SLEEP |
| Category | Control Flow |
| Syntax | SLEEP [seconds] |
| Description | Suspends program execution for the specified duration in seconds. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/control/flow/sleep.c |
