<!--
Title:        SCOPE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/introspection/scope.c
Generated:    no, hand-written
Status:       current
-->

# `SCOPE` Keyword Reference

## Source Header

```c
// FILENAME: scope.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_control_internal.h, exec_internal.h, module.c, s
// NEEDS: libcore (language_descriptor.h, memops.h, strops.h)
// NEEDS: libengine (alias.h, lexer.h, scope.h, vm.h)
// NEEDS: libkernel (types.h, vdev.h)
// Provides runtime implementation for the SCOPE statement in BASIC++.
```

## 1. Description & Usage

Manages lexical scope blocks, keyword enablement, execution hooks, and symbol protection.

## 2. Syntax

```basic
SCOPE [BEGIN | END | DISABLE kw | ENABLE kw | HOOK ... | MODULE name | PRIVATE sym]
```

## 3. Code Example

```basic
10 REM SCOPE Demonstration
20 PRINT "SCOPE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Permission Denied

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
| Name | SCOPE |
| Category | Introspection |
| Syntax | SCOPE [BEGIN \| END \| DISABLE kw \| ENABLE kw \| HOOK ... \| MODULE name \| PRIVATE sym] |
| Description | Manages lexical scope blocks, keyword enablement, execution hooks, and symbol protection. |
| Error Summary | Error 2: Syntax Error, Error 13: Permission Denied |
| Subsystem | SUBSYSTEM_FLEX |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/introspection/scope.c |
