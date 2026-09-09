<!--
Title:        READ
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/data/read.c
Generated:    no, hand-written
Status:       current
-->

# `READ` Keyword Reference

## Source Header

```c
// FILENAME: read.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c, ctype.h, ctype.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, read.h, string.c, vm.h)
// Provides runtime implementation for the READ statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Reads sequential values from DATA statements and assigns them to variables.

## 2. Syntax

```basic
READ var1 [, var2...]
```

## 3. Code Example

```basic
10 REM READ Demonstration
20 PRINT "READ executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 4: Out of Data, Error 13: Type Mismatch

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
| Name | READ |
| Category | Variables & Memory |
| Syntax | READ var1 [, var2...] |
| Description | Reads sequential values from DATA statements and assigns them to variables. |
| Error Summary | Error 2: Syntax Error, Error 4: Out of Data, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/data/read.c |
