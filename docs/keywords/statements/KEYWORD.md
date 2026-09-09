<!--
Title:        KEYWORD
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/introspection/keyword.c
Generated:    no, hand-written
Status:       current
-->

# `KEYWORD` Keyword Reference

## Source Header

```c
// FILENAME: keyword.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_internal.h)
// NEEDS: libcore (keyword_props.h, language_descriptor.h, memops.h, strops.h)
// NEEDS: libengine (keyword.h, lexer.h, vm.h)
// NEEDS: libkernel (types.h, vdev.h)
// Provides runtime implementation for the KEYWORD statement in BASIC++.
```

## 1. Description & Usage

Configures dynamic syntactic and semantic properties for engine keywords.

## 2. Syntax

```basic
KEYWORD target SET prop = val | KEYWORD target GET prop | KEYWORD target LIST | KEYWORD CLEAR
```

## 3. Code Example

```basic
10 REM KEYWORD Demonstration
20 PRINT "KEYWORD executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Introspection
- **Subsystem**: SUBSYSTEM_FLEX
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | KEYWORD |
| Category | Introspection |
| Syntax | KEYWORD target SET prop = val \| KEYWORD target GET prop \| KEYWORD target LIST \| KEYWORD CLEAR |
| Description | Configures dynamic syntactic and semantic properties for engine keywords. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_FLEX |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/introspection/keyword.c |
