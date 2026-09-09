<!--
Title:        DEF
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/declaration/def.c
Generated:    no, hand-written
Status:       current
-->

# `DEF` Keyword Reference

## Source Header

```c
// FILENAME: def.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (eval_expr_internal.h, exec_internal.h, sub_internal.h
// NEEDS: libcore (ctype.h, ctype.c, language_descriptor.h)
// NEEDS: libcore (string.h, strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (def.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the DEF statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Defines a user-defined numeric or string function.

## 2. Syntax

```basic
DEF FNname[(args)] = expr
```

## 3. Code Example

```basic
10 Val = DEF FNname[(args)] = e10pr
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Function Definitions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DEF |
| Category | Function Definitions |
| Syntax | DEF FNname[(args)] = expr |
| Description | Defines a user-defined numeric or string function. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/declaration/def.c |
