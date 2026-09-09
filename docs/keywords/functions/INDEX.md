<!--
Title:        INDEX
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/search/index_fn.c
Generated:    no, hand-written
Status:       current
-->

# `INDEX` Keyword Reference

## Source Header

```c
// FILENAME: index_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (index_fn.h, string.c)
// Provides runtime implementation for INDEX and INDEX$ array and string searc
//
// ---- Includes ----
```

## 1. Description & Usage

Returns 1-based index of target in numeric array, string array, or string.

## 2. Syntax

```basic
INDEX(arr_or_str, target [, start_pos])
```

## 3. Code Example

```basic
10 Val = INDEX(arr_or_str, target [, start_pos])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Array & String Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | INDEX |
| Category | Array & String Functions |
| Syntax | INDEX(arr_or_str, target [, start_pos]) |
| Description | Returns 1-based index of target in numeric array, string array, or string. |
| Error Summary | Error 13: Type Mismatch, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/search/index_fn.c |
