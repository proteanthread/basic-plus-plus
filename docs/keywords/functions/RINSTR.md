<!--
Title:        RINSTR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/search/rinstr.c
Generated:    no, hand-written
Status:       current
-->

# `RINSTR` Keyword Reference

## Source Header

```c
// FILENAME: rinstr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c, strings.h, strings.c)
// NEEDS: libengine (rinstr.h, string.c, vm.h)
// Provides runtime implementation for the RINSTR built-in function in BASIC++
//
// ---- Includes ----
```

## 1. Description & Usage

Returns 1-based position of the last occurrence of match$ in str$ searching backwards.

## 2. Syntax

```basic
RINSTR([start,] str$, match$ [, mode])
```

## 3. Code Example

```basic
10 Val = RINSTR([start,] str$, match$ [, mode])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (expects string arguments)

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
| Name | RINSTR |
| Category | String Functions |
| Syntax | RINSTR([start,] str$, match$ [, mode]) |
| Description | Returns 1-based position of the last occurrence of match$ in str$ searching backwards. |
| Error Summary | Error 13: Type Mismatch (expects string arguments) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/search/rinstr.c |
