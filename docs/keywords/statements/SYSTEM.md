<!--
Title:        SYSTEM
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/system.c
Generated:    no, hand-written
Status:       current
-->

# `SYSTEM` Keyword Reference

## Source Header

```c
// FILENAME: system.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform
// Provides runtime implementation for the SYSTEM statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Exits BASIC++ interpreter session or executes an operating system command.

## 2. Syntax

```basic
SYSTEM | BYE | SHELL [command_string$]
```

## 3. Code Example

```basic
10 REM SYSTEM Demonstration
20 PRINT "SYSTEM executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 70: Permission Denied

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Environ
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | SYSTEM |
| Category | System & Environ |
| Syntax | SYSTEM \| BYE \| SHELL [command_string$] |
| Description | Exits BASIC++ interpreter session or executes an operating system command. |
| Error Summary | Error 2: Syntax Error, Error 70: Permission Denied |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/system.c |
