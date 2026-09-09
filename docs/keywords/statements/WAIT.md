<!--
Title:        WAIT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/control/flow/wait.c
Generated:    no, hand-written
Status:       current
-->

# `WAIT` Keyword Reference

## Source Header

```c
// FILENAME: wait.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, string.c, wait.h)
// NEEDS: libkernel (errors.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the WAIT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Suspends execution for specified duration, or polls hardware port until condition is met.

## 2. Syntax

```basic
WAIT seconds | WAIT port, and_mask [, xor_mask] | WAIT #channel, seconds
```

## 3. Code Example

```basic
10 REM WAIT Demonstration
20 PRINT "WAIT executed successfully."
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
| Name | WAIT |
| Category | Control Flow |
| Syntax | WAIT seconds \| WAIT port, and_mask [, xor_mask] \| WAIT #channel, seconds |
| Description | Suspends execution for specified duration, or polls hardware port until condition is met. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/control/flow/wait.c |
