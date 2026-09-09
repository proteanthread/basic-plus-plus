<!--
Title:        INVOKE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/environment/invoke.c
Generated:    no, hand-written
Status:       current
-->

# `INVOKE` Keyword Reference

## Source Header

```c
// FILENAME: invoke.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, invoke.h, string.c)
// NEEDS: libkernel (security.h, security.c)
// Provides runtime implementation for the INVOKE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Apple /// Business BASIC dynamic driver and module loader.

## 2. Syntax

```basic
INVOKE driver_path$
```

## 3. Code Example

```basic
10 REM INVOKE Demonstration
20 PRINT "INVOKE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch, Error 53: File Not Found

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Modules
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | INVOKE |
| Category | System & Modules |
| Syntax | INVOKE driver_path$ |
| Description | Apple /// Business BASIC dynamic driver and module loader. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch, Error 53: File Not Found |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/environment/invoke.c |
