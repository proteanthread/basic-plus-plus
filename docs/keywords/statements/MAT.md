<!--
Title:        MAT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/matrices/ops/mat_ops.c
Generated:    no, hand-written
Status:       current
-->

# `MAT` Keyword Reference

## Source Header

```c
// FILENAME: mat_ops.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mat_internal.h)
// NEEDS: libengine (mat_internal.h)
// Provides runtime implementation for the MAT_OPS statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Performs matrix operations including addition, subtraction, multiplication, scalar, transpose, and inverse.

## 2. Syntax

```basic
MAT var = expr
```

## 3. Code Example

```basic
10 REM MAT Demonstration
20 PRINT "MAT executed successfully."
```

## 4. Error Conditions

Error 9: Subscript Out of Range, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Matrix Operations
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | MAT |
| Category | Matrix Operations |
| Syntax | MAT var = expr |
| Description | Performs matrix operations including addition, subtraction, multiplication, scalar, transpose, and inverse. |
| Error Summary | Error 9: Subscript Out of Range, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/matrices/ops/mat_ops.c |
