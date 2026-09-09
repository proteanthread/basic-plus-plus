<!--
Title:        CLRBIT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/bits/manipulation/resetbit.c
Generated:    no, hand-written
Status:       current
-->

# `CLRBIT` Keyword Reference

## Source Header

```c
// FILENAME: resetbit.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (resetbit.h)
// Provides runtime implementation for the RESETBIT built-in function in BASIC
//
// ---- Includes ----
```

## 1. Description & Usage

Clears (sets to 0) the bit at the specified zero-based bit index (alias for RESETBIT).

## 2. Syntax

```basic
CLRBIT(val, bit)
```

## 3. Code Example

```basic
10 Val = CLRBIT(val, bit)
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
| Name | CLRBIT |
| Category | Bitwise & Logical Functions |
| Syntax | CLRBIT(val, bit) |
| Description | Clears (sets to 0) the bit at the specified zero-based bit index (alias for RESETBIT). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/bits/manipulation/resetbit.c |
