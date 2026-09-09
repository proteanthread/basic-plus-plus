<!--
Title:        SHUFFLE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/data/stmt_shuffle.c
Generated:    no, hand-written
Status:       current
-->

# `SHUFFLE` Keyword Reference

## Source Header

```c
// FILENAME: stmt_shuffle.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, language_descriptor.h, strings.h, variables.h)
// NEEDS: libengine (eval.h, lexer.h, rnd.h, stmt_shuffle.h, vm.h)
// Provides runtime implementation for the SHUFFLE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Randomly shuffles array elements or string characters in-place using Fisher-Yates algorithm.

## 2. Syntax

```basic
SHUFFLE arr[()] [, seed] | SHUFFLE (arr [, seed]) | SHUFFLE str$ [, seed]
```

## 3. Code Example

```basic
10 Val = SHUFFLE arr[()] [, seed] | SHUFFLE (arr [, seed]) | SHUFFLE str$ [, seed]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Variables & Memory
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | SHUFFLE |
| Category | Variables & Memory |
| Syntax | SHUFFLE arr[()] [, seed] \| SHUFFLE (arr [, seed]) \| SHUFFLE str$ [, seed] |
| Description | Randomly shuffles array elements or string characters in-place using Fisher-Yates algorithm. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/data/stmt_shuffle.c |
