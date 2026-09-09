<!--
Title:        SPACE$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/space.c
Generated:    no, hand-written
Status:       current
-->

# `SPACE$` Keyword Reference

## Source Header

```c
// FILENAME: space.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (space.h, string.c)
// Provides runtime implementation for the SPACE built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns a string consisting of n space characters.

## 2. Syntax

```basic
SPACE$(n)
```

## 3. Code Example

```basic
10 Val = SPACE$(n)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call (n < 0), Error 13: Type Mismatch (SPACE$ expects one numeric argument)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: String Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | SPACE$ |
| Category | String Functions |
| Syntax | SPACE$(n) |
| Description | Returns a string consisting of n space characters. |
| Error Summary | Error 5: Illegal Function Call (n < 0), Error 13: Type Mismatch (SPACE$ expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/space.c |
