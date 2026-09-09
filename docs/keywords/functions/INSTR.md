<!--
Title:        INSTR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/search/instr.c
Generated:    no, hand-written
Status:       current
-->

# `INSTR` Keyword Reference

## Source Header

```c
// FILENAME: instr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (instr.h, string.c)
// Provides runtime implementation for the INSTR built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the 1-based position of match$ in str$ (supports negative start for reverse search).

## 2. Syntax

```basic
INSTR([start,] str$, match$ [, mode])
```

## 3. Code Example

```basic
10 Val = INSTR([start,] str$, match$ [, mode])
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
| Name | INSTR |
| Category | String Functions |
| Syntax | INSTR([start,] str$, match$ [, mode]) |
| Description | Returns the 1-based position of match$ in str$ (supports negative start for reverse search). |
| Error Summary | Error 13: Type Mismatch (expects string arguments) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/search/instr.c |
