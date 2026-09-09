<!--
Title:        GOODBYE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/goodbye.c
Generated:    no, hand-written
Status:       current
-->

# `GOODBYE` Keyword Reference

## Source Header

```c
// FILENAME: goodbye.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform
// Implements the forced session exit statement GOODBYE in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Forcefully terminates all background tasks and aborts the session immediately.

## 2. Syntax

```basic
GOODBYE
```

## 3. Code Example

```basic
10 REM GOODBYE Demonstration
20 PRINT "GOODBYE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Environment
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | GOODBYE |
| Category | System & Environment |
| Syntax | GOODBYE |
| Description | Forcefully terminates all background tasks and aborts the session immediately. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/goodbye.c |
