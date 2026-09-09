<!--
Title:        INSERT$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/func_insert.c
Generated:    no, hand-written
Status:       current
-->

# `INSERT$` Keyword Reference

## Source Header

```c
// FILENAME: func_insert.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libengine (func_insert.h, vm.h)
// NEEDS: libcore (language_descriptor.h, strings.h, string.h, memory.h, array
// Provides runtime implementation for INSERT and INSERT$ in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Inserts elements into numeric arrays, string arrays, strings, or dynamic arrays.

## 2. Syntax

```basic
INSERT$(target$, source$, pos) | INSERT(arr, pos, val) | INSERT$(dyn$, attr, val$)
```

## 3. Code Example

```basic
10 Val = INSERT$(target$, source$, pos) | INSERT(arr, pos, val) | INSERT$(dyn$, attr, val$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch, Error 7: Out of Memory

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
| Name | INSERT$ |
| Category | Array & String Functions |
| Syntax | INSERT$(target$, source$, pos) \| INSERT(arr, pos, val) \| INSERT$(dyn$, attr, val$) |
| Description | Inserts elements into numeric arrays, string arrays, strings, or dynamic arrays. |
| Error Summary | Error 13: Type Mismatch, Error 7: Out of Memory |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/func_insert.c |
