<!--
Title:        SHELL
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/environment/shell.c
Generated:    no, hand-written
Status:       current
-->

# `SHELL` Keyword Reference

## Source Header

```c
// FILENAME: shell.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, shell.h, string.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the SHELL statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Suspends the BASIC program and executes an operating system command shell.

## 2. Syntax

```basic
SHELL [command_string$]
```

## 3. Code Example

```basic
10 REM SHELL Demonstration
20 PRINT "SHELL executed successfully."
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
| Name | SHELL |
| Category | System & OS |
| Syntax | SHELL [command_string$] |
| Description | Suspends the BASIC program and executes an operating system command shell. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/environment/shell.c |
