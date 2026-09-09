<!--
Title:        ROL
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/bits/shift/rol.c
Generated:    no, hand-written
Status:       current
-->

# `ROL` Keyword Reference

## Source Header

```c
// FILENAME: rol.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (rol.h)
// Provides runtime implementation for the ROL built-in function in BASIC++.
```

## 1. Description & Usage

Rotates bits of an integer left by the specified count within the given bit width (default: 64).

## 2. Syntax

```basic
ROL(val, count [, width])
```

## 3. Code Example

```basic
10 Val = ROL(val, count [, width])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Bitwise & Logical Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | ROL |
| Category | Bitwise & Logical Functions |
| Syntax | ROL(val, count [, width]) |
| Description | Rotates bits of an integer left by the specified count within the given bit width (default: 64). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/bits/shift/rol.c |
