<!--
Title:        CSTR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/type.c
Generated:    no, hand-written
Status:       current
-->

# `CSTR` Keyword Reference

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

Converts expression of any type to its standard string representation.

## 2. Syntax

```basic
CSTR(expr)
```

## 3. Code Example

```basic
10 Val = CSTR(e10pr)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 14: Out of String Space

## 5. Compatibility & Lineage

- **Lineage**: VB for DOS, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Type Conversion
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CSTR |
| Category | Type Conversion |
| Syntax | CSTR(expr) |
| Description | Converts expression of any type to its standard string representation. |
| Error Summary | Error 14: Out of String Space |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | VB for DOS, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/eval/type.c |
