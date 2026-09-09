<!--
Title:        CINT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/type.c
Generated:    no, hand-written
Status:       current
-->

# `CINT` Keyword Reference

## Source Header

```c
// FILENAME: type.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (type.h)
// Provides core logic and interface definitions for type within BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Converts numeric expression to 16-bit or 32-bit integer, rounding fractional part (banker's rounding).

## 2. Syntax

```basic
CINT(expr)
```

## 3. Code Example

```basic
10 Val = CINT(e10pr)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 6: Overflow, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: GW-BASIC, QBASIC, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Type Conversion
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CINT |
| Category | Type Conversion |
| Syntax | CINT(expr) |
| Description | Converts numeric expression to 16-bit or 32-bit integer, rounding fractional part (banker's rounding). |
| Error Summary | Error 6: Overflow, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | GW-BASIC, QBASIC, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/eval/type.c |
