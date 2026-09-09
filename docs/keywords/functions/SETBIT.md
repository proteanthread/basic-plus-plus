<!--
Title:        SETBIT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/bits/manipulation/setbit.c
Generated:    no, hand-written
Status:       current
-->

# `SETBIT` Keyword Reference

## Source Header

```c
// FILENAME: setbit.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (setbit.h)
// Provides runtime implementation for the SETBIT built-in function in BASIC++
//
// ---- Includes ----
```

## 1. Description & Usage

Sets (to 1) the bit at the specified zero-based bit index in an integer value.

## 2. Syntax

```basic
SETBIT(val, bit)
```

## 3. Code Example

```basic
10 Val = SETBIT(val, bit)
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
| Name | SETBIT |
| Category | Bitwise & Logical Functions |
| Syntax | SETBIT(val, bit) |
| Description | Sets (to 1) the bit at the specified zero-based bit index in an integer value. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/bits/manipulation/setbit.c |
