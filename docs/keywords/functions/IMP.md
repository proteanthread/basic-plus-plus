<!--
Title:        IMP
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/bits/logic/imp.c
Generated:    no, hand-written
Status:       current
-->

# `IMP` Keyword Reference

## Source Header

```c
// FILENAME: imp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (imp.h)
// Provides runtime implementation for the IMP built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Performs bitwise and logical implication (NOT val1 OR val2) on integers or boolean values.

## 2. Syntax

```basic
IMP(val1, val2) or val1 IMP val2
```

## 3. Code Example

```basic
10 Val = IMP(val1, val2) or val1 IMP val2
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
| Name | IMP |
| Category | Bitwise & Logical Functions |
| Syntax | IMP(val1, val2) or val1 IMP val2 |
| Description | Performs bitwise and logical implication (NOT val1 OR val2) on integers or boolean values. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/bits/logic/imp.c |
