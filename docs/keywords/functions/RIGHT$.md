<!--
Title:        RIGHT$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/right.c
Generated:    no, hand-written
Status:       current
-->

# `RIGHT$` Keyword Reference

## Source Header

```c
// FILENAME: right.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (right.h, string.c, vm.h)
// Provides runtime implementation for the RIGHT built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the rightmost n characters of str$.

## 2. Syntax

```basic
RIGHT$(str$, n)
```

## 3. Code Example

```basic
10 Val = RIGHT$(str$, n)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call (n < 0), Error 13: Type Mismatch (RIGHT$ expects string and numeric arguments)

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
| Name | RIGHT$ |
| Category | String Functions |
| Syntax | RIGHT$(str$, n) |
| Description | Returns the rightmost n characters of str$. |
| Error Summary | Error 5: Illegal Function Call (n < 0), Error 13: Type Mismatch (RIGHT$ expects string and numeric arguments) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/right.c |
