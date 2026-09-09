<!--
Title:        LSET
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/assignment/lset.c
Generated:    no, hand-written
Status:       current
-->

# `LSET` Keyword Reference

## Source Header

```c
// FILENAME: lset.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h, strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, lset.h, string.c, vm.h)
// Provides runtime implementation for the LSET statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Left justifies a string in a fixed-length string variable or FIELD buffer.

## 2. Syntax

```basic
LSET string_var = string_expression
```

## 3. Code Example

```basic
10 REM LSET Demonstration
20 PRINT "LSET executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

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
| Name | LSET |
| Category | Variables & Memory |
| Syntax | LSET string_var = string_expression |
| Description | Left justifies a string in a fixed-length string variable or FIELD buffer. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/assignment/lset.c |
