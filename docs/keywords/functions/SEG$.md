<!--
Title:        SEG$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/seg.c
Generated:    no, hand-written
Status:       current
-->

# `SEG$` Keyword Reference

## Source Header

```c
// FILENAME: seg.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (seg.h, string.c)
// Provides runtime implementation for the SEG built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns a substring from 1-based start_pos through end_pos inclusive (DEC BASIC-PLUS).

## 2. Syntax

```basic
SEG$(str_expr, start_pos, end_pos)
```

## 3. Code Example

```basic
10 Val = SEG$(str_e10pr, start_pos, end_pos)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch, Error 5: Illegal Function Call

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
| Name | SEG$ |
| Category | String Functions |
| Syntax | SEG$(str_expr, start_pos, end_pos) |
| Description | Returns a substring from 1-based start_pos through end_pos inclusive (DEC BASIC-PLUS). |
| Error Summary | Error 13: Type Mismatch, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/seg.c |
