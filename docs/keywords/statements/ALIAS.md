<!--
Title:        ALIAS
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/introspection/alias.c
Generated:    no, hand-written
Status:       current
-->

# `ALIAS` Keyword Reference

## Source Header

```c
// FILENAME: alias.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_internal.h, scope.c)
// NEEDS: libcore (hal.h, memops.h, strops.h, runtime_snprintf.h)
// NEEDS: libengine (alias.h, lexer.h, vm.h)
// NEEDS: libkernel (types.h, vdev.h)
// Provides runtime implementation for the ALIAS statement in BASIC++.
```

## 1. Description & Usage

Defines a keyword or operator alias for customized dialect syntax.

## 2. Syntax

```basic
ALIAS name = expansion | ALIAS LIST | ALIAS CLEAR | ALIAS OPER op = expansion
```

## 3. Code Example

```basic
10 REM ALIAS Demonstration
20 PRINT "ALIAS executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 53: File Not Found

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
| Name | ALIAS |
| Category | Introspection |
| Syntax | ALIAS name = expansion \| ALIAS LIST \| ALIAS CLEAR \| ALIAS OPER op = expansion |
| Description | Defines a keyword or operator alias for customized dialect syntax. |
| Error Summary | Error 2: Syntax Error, Error 53: File Not Found |
| Subsystem | SUBSYSTEM_FLEX |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/introspection/alias.c |
