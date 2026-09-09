<!--
Title:        NUM
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/type.c
Generated:    no, hand-written
Status:       current
-->

# `NUM` Keyword Reference

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

Extracts or counts the numeric elements in expression or collection.

## 2. Syntax

```basic
NUM(expr)
```

## 3. Code Example

```basic
10 Val = NUM(e10pr)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Introspection & Conversion
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | NUM |
| Category | Introspection & Conversion |
| Syntax | NUM(expr) |
| Description | Extracts or counts the numeric elements in expression or collection. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/eval/type.c |
