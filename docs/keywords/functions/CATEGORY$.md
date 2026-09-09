<!--
Title:        CATEGORY$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/environment/category.c
Generated:    no, hand-written
Status:       current
-->

# `CATEGORY$` Keyword Reference

## Source Header

```c
// FILENAME: category.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (funcreg.h, language_descriptor.h, spec.h, string.h, strings
// Provides runtime implementation for the CATEGORY$ built-in function in BASI
```

## 1. Description & Usage

Returns the category name of the specified keyword, or empty string if not found.

## 2. Syntax

```basic
CATEGORY$(keyword$)
```

## 3. Code Example

```basic
10 Val = CATEGORY$(keyword$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (expects one string argument)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Introspection
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CATEGORY$ |
| Category | Introspection |
| Syntax | CATEGORY$(keyword$) |
| Description | Returns the category name of the specified keyword, or empty string if not found. |
| Error Summary | Error 13: Type Mismatch (expects one string argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/environment/category.c |
