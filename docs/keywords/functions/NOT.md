<!--
Title:        NOT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/bits/logic/not.c
Generated:    no, hand-written
Status:       current
-->

# `NOT` Keyword Reference

## Source Header

```c
// FILENAME: not.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (not.h)
// Provides runtime implementation for the NOT built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Performs bitwise and logical negation on an integer or boolean value.

## 2. Syntax

```basic
NOT(val) or NOT val
```

## 3. Code Example

```basic
10 Val = NOT(val) or NOT val
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
| Name | NOT |
| Category | Bitwise & Logical Functions |
| Syntax | NOT(val) or NOT val |
| Description | Performs bitwise and logical negation on an integer or boolean value. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/bits/logic/not.c |
