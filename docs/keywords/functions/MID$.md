<!--
Title:        MID$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/mid.c
Generated:    no, hand-written
Status:       current
-->

# `MID$` Keyword Reference

## Source Header

```c
// FILENAME: mid.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (mid.h, string.c, vm.h)
// Provides runtime implementation for the MID built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns a substring of str$ with optional negative start/length and strided step slicing.

## 2. Syntax

```basic
MID$(str$, start [, length [, step]])
```

## 3. Code Example

```basic
10 Val = MID$(str$, start [, length [, step]])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (MID$ argument type error)

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
| Name | MID$ |
| Category | String Functions |
| Syntax | MID$(str$, start [, length [, step]]) |
| Description | Returns a substring of str$ with optional negative start/length and strided step slicing. |
| Error Summary | Error 13: Type Mismatch (MID$ argument type error) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/mid.c |
