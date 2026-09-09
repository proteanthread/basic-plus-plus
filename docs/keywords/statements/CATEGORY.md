<!--
Title:        CATEGORY
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/introspection/category.c
Generated:    no, hand-written
Status:       current
-->

# `CATEGORY` Keyword Reference

## Source Header

```c
// FILENAME: category.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libflex
// NEEDS: libcore (funcreg.h, language_descriptor.h, memops.h, spec.h, strops.
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the CATEGORY statement in BASIC++.
```

## 1. Description & Usage

Displays all categories, queries the category of a keyword, or lists all keywords in a category.

## 2. Syntax

```basic
CATEGORY [keyword | category_name]
```

## 3. Code Example

```basic
10 REM CATEGORY Demonstration
20 PRINT "CATEGORY executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Introspection
- **Subsystem**: SUBSYSTEM_CORE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CATEGORY |
| Category | Introspection |
| Syntax | CATEGORY [keyword \| category_name] |
| Description | Displays all categories, queries the category of a keyword, or lists all keywords in a category. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_CORE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/introspection/category.c |
