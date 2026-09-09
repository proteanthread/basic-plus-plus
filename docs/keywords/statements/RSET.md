<!--
Title:        RSET
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/assignment/rset.c
Generated:    no, hand-written
Status:       current
-->

# `RSET` Keyword Reference

## Source Header

```c
// FILENAME: rset.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h, strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, rset.h, string.c)
// Provides runtime implementation for the RSET statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Right justifies a string in a fixed-length string variable or FIELD buffer.

## 2. Syntax

```basic
RSET string_var = string_expression
```

## 3. Code Example

```basic
10 REM RSET Demonstration
20 PRINT "RSET executed successfully."
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
| Name | RSET |
| Category | Variables & Memory |
| Syntax | RSET string_var = string_expression |
| Description | Right justifies a string in a fixed-length string variable or FIELD buffer. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/assignment/rset.c |
