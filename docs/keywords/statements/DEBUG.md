<!--
Title:        DEBUG
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/debug.c
Generated:    no, hand-written
Status:       current
-->

# `DEBUG` Keyword Reference

## Source Header

```c
// FILENAME: debug.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel
// Provides runtime implementation for the DEBUG statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Master interactive debugger control, execution tracing, breakpoint handling, and system introspection suite.

## 2. Syntax

```basic
DEBUG [ON|OFF|DUMP|STACK|MEMORY|VARS] | TRACE | TRON | TROFF | BREAK | CONT | BACKTRACE | INFO | DUMP
```

## 3. Code Example

```basic
10 REM DEBUG Demonstration
20 PRINT "DEBUG executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 99: Assertion Failed

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Debug & Testing
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DEBUG |
| Category | Debug & Testing |
| Syntax | DEBUG [ON\|OFF\|DUMP\|STACK\|MEMORY\|VARS] \| TRACE \| TRON \| TROFF \| BREAK \| CONT \| BACKTRACE \| INFO \| DUMP |
| Description | Master interactive debugger control, execution tracing, breakpoint handling, and system introspection suite. |
| Error Summary | Error 2: Syntax Error, Error 99: Assertion Failed |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/debug.c |
