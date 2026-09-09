<!--
Title:        ERROR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/vm/error/error.c
Generated:    no, hand-written
Status:       current
-->

# `ERROR` Keyword Reference

## Source Header

```c
// FILENAME: error.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for ERROR.
```

## 1. Description & Usage

Simulates a runtime error with the specified numeric code, triggering active ON ERROR traps.

## 2. Syntax

```basic
ERROR error_code%
```

## 3. Code Example

```basic
10 REM ERROR Demonstration
20 PRINT "ERROR executed successfully."
```

## 4. Error Conditions

Simulates runtime error code

## 5. Compatibility & Lineage

- **Lineage**: GW-BASIC, QBASIC, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Error Handling
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | ERROR |
| Category | Error Handling |
| Syntax | ERROR error_code% |
| Description | Simulates a runtime error with the specified numeric code, triggering active ON ERROR traps. |
| Error Summary | Simulates runtime error code |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | GW-BASIC, QBASIC, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/vm/error/error.c |
