<!--
Title:        PUBLIC
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/declaration/public.c
Generated:    no, hand-written
Status:       current
-->

# `PUBLIC` Keyword Reference

## Source Header

```c
// FILENAME: public.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (lexer.h, lexer.c, public.h, string.c, vm.h)
// Provides runtime implementation for the PUBLIC statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Exports specified variables or procedures from the current module scope to global scope.

## 2. Syntax

```basic
PUBLIC [SUB|FUNCTION] name [, name...]
```

## 3. Code Example

```basic
10 REM PUBLIC Demonstration
20 PRINT "PUBLIC executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Variables & Memory
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | PUBLIC |
| Category | Variables & Memory |
| Syntax | PUBLIC [SUB\|FUNCTION] name [, name...] |
| Description | Exports specified variables or procedures from the current module scope to global scope. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/declaration/public.c |
