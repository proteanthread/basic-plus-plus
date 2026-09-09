<!--
Title:        RETRY
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/control/flow/retry.c
Generated:    no, hand-written
Status:       current
-->

# `RETRY` Keyword Reference

## Source Header

```c
// FILENAME: retry.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (lexer.h, lexer.c, resume.h, resume.c, retry.h, string.c)
// NEEDS: libengine (vm.h)
// Provides runtime implementation for the RETRY statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

ECMA-116 standard statement to re-execute the statement that caused an exception.

## 2. Syntax

```basic
RETRY
```

## 3. Code Example

```basic
10 REM RETRY Demonstration
20 PRINT "RETRY executed successfully."
```

## 4. Error Conditions

Error 20: RESUME Without Error

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
| Name | RETRY |
| Category | Control Flow |
| Syntax | RETRY |
| Description | ECMA-116 standard statement to re-execute the statement that caused an exception. |
| Error Summary | Error 20: RESUME Without Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/control/flow/retry.c |
