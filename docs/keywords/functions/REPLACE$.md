<!--
Title:        REPLACE$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/func_replace.c
Generated:    no, hand-written
Status:       current
-->

# `REPLACE$` Keyword Reference

## Source Header

```c
// FILENAME: func_replace.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libengine (func_replace.h, vm.h)
// NEEDS: libcore (language_descriptor.h, strings.h, string.h, memory.h, array
// Provides runtime implementation for REPLACE and REPLACE$ in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Replaces elements in numeric arrays, string arrays, strings, or dynamic arrays.

## 2. Syntax

```basic
REPLACE$(target$, search$, replace$ [, count [, mode]]) | REPLACE(arr, pos, val) | REPLACE$(dyn$, attr, val$)
```

## 3. Code Example

```basic
10 Val = REPLACE$(target$, search$, replace$ [, count [, mode]]) | REPLACE(arr, pos, val) | REPLACE$(dyn$, attr, val$)
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
| Name | REPLACE$ |
| Category | Array & String Functions |
| Syntax | REPLACE$(target$, search$, replace$ [, count [, mode]]) \| REPLACE(arr, pos, val) \| REPLACE$(dyn$, attr, val$) |
| Description | Replaces elements in numeric arrays, string arrays, strings, or dynamic arrays. |
| Error Summary | Error 13: Type Mismatch, Error 7: Out of Memory |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/func_replace.c |
