<!--
Title:        DELETE$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/func_delete_str.c
Generated:    no, hand-written
Status:       current
-->

# `DELETE$` Keyword Reference

## Source Header

```c
// FILENAME: func_delete_str.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libengine (func_delete_str.h, vm.h)
// NEEDS: libcore (language_descriptor.h, strings.h, string.h, memory.h, array
// Provides runtime implementation for DELETE and DELETE$ in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Deletes elements from numeric arrays, string arrays, strings, or dynamic arrays.

## 2. Syntax

```basic
DELETE$(target$, start, count) | DELETE(arr, pos [, count]) | DELETE$(dyn$, attr)
```

## 3. Code Example

```basic
10 Val = DELETE$(target$, start, count) | DELETE(arr, pos [, count]) | DELETE$(dyn$, attr)
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
| Name | DELETE$ |
| Category | Array & String Functions |
| Syntax | DELETE$(target$, start, count) \| DELETE(arr, pos [, count]) \| DELETE$(dyn$, attr) |
| Description | Deletes elements from numeric arrays, string arrays, strings, or dynamic arrays. |
| Error Summary | Error 13: Type Mismatch, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/func_delete_str.c |
