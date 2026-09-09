<!--
Title:        TOGGLEBIT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/bits/manipulation/togglebit.c
Generated:    no, hand-written
Status:       current
-->

# `TOGGLEBIT` Keyword Reference

## Source Header

```c
// FILENAME: togglebit.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (togglebit.h)
// Provides runtime implementation for the TOGGLEBIT built-in function in BASI
//
// ---- Includes ----
```

## 1. Description & Usage

Inverts (flips) the bit at the specified zero-based bit index in an integer value.

## 2. Syntax

```basic
TOGGLEBIT(val, bit)
```

## 3. Code Example

```basic
10 Val = TOGGLEBIT(val, bit)
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
| Name | TOGGLEBIT |
| Category | Bitwise & Logical Functions |
| Syntax | TOGGLEBIT(val, bit) |
| Description | Inverts (flips) the bit at the specified zero-based bit index in an integer value. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/bits/manipulation/togglebit.c |
