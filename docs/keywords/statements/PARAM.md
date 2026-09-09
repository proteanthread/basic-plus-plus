<!--
Title:        PARAM
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/oop/structure/param.c
Generated:    no, hand-written
Status:       current
-->

# `PARAM` Keyword Reference

## Source Header

```c
// FILENAME: param.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (lexer.h, lexer.c, param.h, string.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the PARAM statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Declares and binds formal parameter variables inside a BASIC09 PROCEDURE block.

## 2. Syntax

```basic
PARAM var1 [: type] [, var2 [: type] ...]
```

## 3. Code Example

```basic
10 REM PARAM Demonstration
20 PRINT "PARAM executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Procedures / OOP
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | PARAM |
| Category | Procedures / OOP |
| Syntax | PARAM var1 [: type] [, var2 [: type] ...] |
| Description | Declares and binds formal parameter variables inside a BASIC09 PROCEDURE block. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/oop/structure/param.c |
