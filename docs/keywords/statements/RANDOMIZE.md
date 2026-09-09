<!--
Title:        RANDOMIZE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/options/randomize.c
Generated:    no, hand-written
Status:       current
-->

# `RANDOMIZE` Keyword Reference

## Source Header

```c
// FILENAME: randomize.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (lexer.h, lexer.c, randomize.h, string.c, vm.h)
// Provides runtime implementation for the RANDOMIZE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Reseeds the pseudo-random number generator used by the RND function.

## 2. Syntax

```basic
RANDOMIZE [seed]
```

## 3. Code Example

```basic
10 REM RANDOMIZE Demonstration
20 PRINT "RANDOMIZE executed successfully."
```

## 4. Error Conditions

Error 5: Illegal Function Call (negative or invalid seed)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Control & Math
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | RANDOMIZE |
| Category | Control & Math |
| Syntax | RANDOMIZE [seed] |
| Description | Reseeds the pseudo-random number generator used by the RND function. |
| Error Summary | Error 5: Illegal Function Call (negative or invalid seed) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/options/randomize.c |
