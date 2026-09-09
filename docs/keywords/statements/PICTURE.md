<!--
Title:        PICTURE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/bgi/picture.c
Generated:    no, hand-written
Status:       current
-->

# `PICTURE` Keyword Reference

## Source Header

```c
// FILENAME: picture.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (lexer.h, lexer.c, picture.h, string.c, vm.h)
// Provides runtime implementation for the PICTURE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

ECMA-116 standard statement to define a parameterized vector graphic picture macro block.

## 2. Syntax

```basic
PICTURE name [(parameter_list)] ... END PICTURE
```

## 3. Code Example

```basic
10 Val = PICTURE name [(parameter_list)] ... END PICTURE
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Graphics & Sound
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | PICTURE |
| Category | Graphics & Sound |
| Syntax | PICTURE name [(parameter_list)] ... END PICTURE |
| Description | ECMA-116 standard statement to define a parameterized vector graphic picture macro block. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/bgi/picture.c |
