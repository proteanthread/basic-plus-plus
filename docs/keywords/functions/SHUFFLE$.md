<!--
Title:        SHUFFLE$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/shuffle.c
Generated:    no, hand-written
Status:       current
-->

# `SHUFFLE$` Keyword Reference

## Source Header

```c
// FILENAME: shuffle.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (shuffle.h, string.c)
// Provides runtime implementation for the SHUFFLE built-in function in BASIC+
//
// ---- Includes ----
```

## 1. Description & Usage

Returns a copy of str$ with all characters randomly scrambled using Fisher-Yates shuffle.

## 2. Syntax

```basic
SHUFFLE$(str$ [, seed])
```

## 3. Code Example

```basic
10 Val = SHUFFLE$(str$ [, seed])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (SHUFFLE$ expects string first argument, optional numeric seed)

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
| Name | SHUFFLE$ |
| Category | String Functions |
| Syntax | SHUFFLE$(str$ [, seed]) |
| Description | Returns a copy of str$ with all characters randomly scrambled using Fisher-Yates shuffle. |
| Error Summary | Error 13: Type Mismatch (SHUFFLE$ expects string first argument, optional numeric seed) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/shuffle.c |
