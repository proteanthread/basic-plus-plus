<!--
Title:        AND
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/bits/logic/and.c
Generated:    no, hand-written
Status:       current
-->

# `AND` Keyword Reference

## Source Header

```c
// FILENAME: and.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (and.h)
// Provides runtime implementation for the AND built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Performs bitwise and logical AND conjunction on integers or boolean values (supports dual prefix & infix notation).

## 2. Syntax

```basic
AND(val1, val2 [, ...]) or val1 AND val2
```

## 3. Code Example

```basic
10 Val = AND(val1, val2 [, ...]) or val1 AND val2
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (AND expects numeric arguments)

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
| Name | AND |
| Category | Bitwise & Logical Functions |
| Syntax | AND(val1, val2 [, ...]) or val1 AND val2 |
| Description | Performs bitwise and logical AND conjunction on integers or boolean values (supports dual prefix & infix notation). |
| Error Summary | Error 13: Type Mismatch (AND expects numeric arguments) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/bits/logic/and.c |
