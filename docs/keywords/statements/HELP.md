<!--
Title:        HELP
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/introspection/help.c
Generated:    no, hand-written
Status:       current
-->

# `HELP` Keyword Reference

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

Displays interactive help documentation for statements, functions, syntax, and system components.

## 2. Syntax

```basic
HELP [keyword | command | block_target]
```

## 3. Code Example

```basic
10 REM HELP Demonstration
20 PRINT "HELP executed successfully."
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
| Name | HELP |
| Category | Introspection |
| Syntax | HELP [keyword \| command \| block_target] |
| Description | Displays interactive help documentation for statements, functions, syntax, and system components. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_CORE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/introspection/help.c |
