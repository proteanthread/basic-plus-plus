<!--
Title:        CBYTE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/func_systems_types.c
Generated:    no, hand-written
Status:       current
-->

# `CBYTE` Keyword Reference

## Source Header

```c
// FILENAME: func_systems_types.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (dispatch_call.c)
// NEEDS: libcore (language_descriptor.h), libengine (func_systems_types.h)
// Provides runtime function implementation for systems types, pointers, and m
//
// ---- Includes ----
```

## 1. Description & Usage

Converts and clamps value to an 8-bit unsigned byte (0-255).

## 2. Syntax

```basic
b = CBYTE(x) | BYTE(x)
```

## 3. Code Example

```basic
10 Val = b = CBYTE(10) | BYTE(10)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Systems Types
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CBYTE |
| Category | Systems Types |
| Syntax | b = CBYTE(x) \| BYTE(x) |
| Description | Converts and clamps value to an 8-bit unsigned byte (0-255). |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/func_systems_types.c |
