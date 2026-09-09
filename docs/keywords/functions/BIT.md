<!--
Title:        BIT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/bits/manipulation/readbit.c
Generated:    no, hand-written
Status:       current
-->

# `BIT` Keyword Reference

## Source Header

```c
// FILENAME: readbit.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (readbit.h)
// Provides runtime implementation for the READBIT built-in function in BASIC+
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the boolean bit value (0 or 1) at the specified zero-based bit index (HP Series 80).

## 2. Syntax

```basic
BIT(val, bit)
```

## 3. Code Example

```basic
10 Val = BIT(val, bit)
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
| Name | BIT |
| Category | Bitwise & Logical Functions |
| Syntax | BIT(val, bit) |
| Description | Returns the boolean bit value (0 or 1) at the specified zero-based bit index (HP Series 80). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/bits/manipulation/readbit.c |
