<!--
Title:        DEMAND
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/io/input/demand.c
Generated:    no, hand-written
Status:       current
-->

# `DEMAND` Keyword Reference

## Source Header

```c
// FILENAME: demand.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c)
// NEEDS: libcore, libengine, libkernel, libplatform
// Provides runtime implementation for DEMAND statement (JOSS / RAND P-2922).
//
// ---- Includes ----
```

## 1. Description & Usage

Prompts for variable values with automatic '<var> = ' naming and optional constraint validation (JOSS / RAND P-2922).

## 2. Syntax

```basic
DEMAND var1 [, var2 ...] [IN min TO max] [DEFAULT def_val]
```

## 3. Code Example

```basic
10 REM DEMAND Demonstration
20 PRINT "DEMAND executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Console I/O
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DEMAND |
| Category | Console I/O |
| Syntax | DEMAND var1 [, var2 ...] [IN min TO max] [DEFAULT def_val] |
| Description | Prompts for variable values with automatic '<var> = ' naming and optional constraint validation (JOSS / RAND P-2922). |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/io/input/demand.c |
