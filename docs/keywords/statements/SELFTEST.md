<!--
Title:        SELFTEST
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/introspection/selftest.c
Generated:    no, hand-written
Status:       current
-->

# `SELFTEST` Keyword Reference

## Source Header

```c
// FILENAME: selftest.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, language_descriptor.h
// Provides runtime implementation for the SELFTEST statement in BASIC++.
```

## 1. Description & Usage

Executes internal diagnostic suite verifying lexer, memory, string, variable, and array subsystems.

## 2. Syntax

```basic
SELFTEST
```

## 3. Code Example

```basic
10 REM SELFTEST Demonstration
20 PRINT "SELFTEST executed successfully."
```

## 4. Error Conditions

Error 1001-5003: Subsystem Diagnostic Failure

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System
- **Subsystem**: SUBSYSTEM_CORE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | SELFTEST |
| Category | System |
| Syntax | SELFTEST |
| Description | Executes internal diagnostic suite verifying lexer, memory, string, variable, and array subsystems. |
| Error Summary | Error 1001-5003: Subsystem Diagnostic Failure |
| Subsystem | SUBSYSTEM_CORE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/introspection/selftest.c |
