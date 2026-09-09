<!--
Title:        VAR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/variable/var.c
Generated:    no, hand-written
Status:       current
-->

# `VAR` Keyword Reference

## Source Header

```c
// FILENAME: var.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for VAR.
```

## 1. Description & Usage

Declares a block-scoped typed variable with optional immediate initialization expression.

## 2. Syntax

```basic
VAR [DIM] varname [AS type] [= expr]
```

## 3. Code Example

```basic
10 REM VAR Demonstration
20 PRINT "VAR executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Variables & Memory
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | VAR |
| Category | Variables & Memory |
| Syntax | VAR [DIM] varname [AS type] [= expr] |
| Description | Declares a block-scoped typed variable with optional immediate initialization expression. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/variable/var.c |
