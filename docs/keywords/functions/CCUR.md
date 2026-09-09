<!--
Title:        CCUR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/type.c
Generated:    no, hand-written
Status:       current
-->

# `CCUR` Keyword Reference

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

Converts numeric expression to fixed-point currency value (scaled 64-bit integer with 4 decimal digits).

## 2. Syntax

```basic
CCUR(expr)
```

## 3. Code Example

```basic
10 Val = CCUR(e10pr)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 6: Overflow, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: QBASIC, VB for DOS, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Type Conversion
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CCUR |
| Category | Type Conversion |
| Syntax | CCUR(expr) |
| Description | Converts numeric expression to fixed-point currency value (scaled 64-bit integer with 4 decimal digits). |
| Error Summary | Error 6: Overflow, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | QBASIC, VB for DOS, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/eval/type.c |
