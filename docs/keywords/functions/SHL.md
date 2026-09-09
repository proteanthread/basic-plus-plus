<!--
Title:        SHL
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/bits/shift/shl.c
Generated:    no, hand-written
Status:       current
-->

# `SHL` Keyword Reference

## Source Header

```c
// FILENAME: shl.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (shl.h)
// Provides runtime implementation for the SHL built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Shifts an integer value left by the specified bit count.

## 2. Syntax

```basic
SHL(val, count) or val SHL count
```

## 3. Code Example

```basic
10 Val = SHL(val, count) or val SHL count
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
| Name | SHL |
| Category | Bitwise & Logical Functions |
| Syntax | SHL(val, count) or val SHL count |
| Description | Shifts an integer value left by the specified bit count. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/bits/shift/shl.c |
