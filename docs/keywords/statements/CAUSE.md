<!--
Title:        CAUSE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/control/branch/cause.c
Generated:    no, hand-written
Status:       current
-->

# `CAUSE` Keyword Reference

## Source Header

```c
// FILENAME: cause.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (cause.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h
// Provides runtime implementation for the CAUSE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

ECMA-116 standard statement to raise a runtime error.

## 2. Syntax

```basic
CAUSE ERROR error_code
```

## 3. Code Example

```basic
10 REM CAUSE Demonstration
20 PRINT "CAUSE executed successfully."
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
| Name | CAUSE |
| Category | Control Flow |
| Syntax | CAUSE ERROR error_code |
| Description | ECMA-116 standard statement to raise a runtime error. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/control/branch/cause.c |
