<!--
Title:        MODULE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/oop/module.c
Generated:    no, hand-written
Status:       current
-->

# `MODULE` Keyword Reference

## Source Header

```c
// FILENAME: module.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_internal.h, help.c, system.c)
// NEEDS: libcore (ctype.h, ctype.c, language_descriptor.h)
// NEEDS: libcore (module.h, string.h)
// NEEDS: libengine (lexer.h, lexer.c, scope.h, scope.c, stmt.h, string.c)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the MODULE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Defines a module scope namespace, or loads/unloads dynamic extension modules.

## 2. Syntax

```basic
MODULE "ModuleName" [EXPORTS sym1, sym2, ...]
```

## 3. Code Example

```basic
10 REM MODULE Demonstration
20 PRINT "MODULE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 53: File Not Found, Error 70: Permission Denied

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Introspection
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | MODULE |
| Category | Introspection |
| Syntax | MODULE "ModuleName" [EXPORTS sym1, sym2, ...] |
| Description | Defines a module scope namespace, or loads/unloads dynamic extension modules. |
| Error Summary | Error 2: Syntax Error, Error 53: File Not Found, Error 70: Permission Denied |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/oop/module.c |
