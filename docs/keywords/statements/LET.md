<!--
Title:        LET
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/assignment/let.c
Generated:    no, hand-written
Status:       current
-->

# `LET` Keyword Reference

## Source Header

```c
// FILENAME: let.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine
// Provides runtime implementation for the LET statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Assigns the value of an expression to a variable or array element.

## 2. Syntax

```basic
[LET] variable = expression
```

## 3. Code Example

```basic
10 REM LET Demonstration
20 PRINT "LET executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Variables & Memory
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | LET |
| Category | Variables & Memory |
| Syntax | [LET] variable = expression |
| Description | Assigns the value of an expression to a variable or array element. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/assignment/let.c |
