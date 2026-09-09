<!--
Title:        CATALOG
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/introspection/help.c
Generated:    no, hand-written
Status:       current
-->

# `CATALOG` Keyword Reference

## Source Header

```c
// FILENAME: help.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, help.h, spec.h)
// Provides runtime implementation for the HELP and CATALOG statements in BASI
```

## 1. Description & Usage

Displays interactive catalog listing of keyword categories, dynamic modules, global labels, custom blocks, and specifications.

## 2. Syntax

```basic
CATALOG [category_name | keyword1 [, keyword2...]]
```

## 3. Code Example

```basic
10 REM CATALOG Demonstration
20 PRINT "CATALOG executed successfully."
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
| Name | CATALOG |
| Category | Introspection |
| Syntax | CATALOG [category_name \| keyword1 [, keyword2...]] |
| Description | Displays interactive catalog listing of keyword categories, dynamic modules, global labels, custom blocks, and specifications. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_CORE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/introspection/help.c |
