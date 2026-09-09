<!--
Title:        DYNARRAY$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/pick.c
Generated:    no, hand-written
Status:       current
-->

# `DYNARRAY$` Keyword Reference

## Source Header

```c
// FILENAME: pick.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (pick.h, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for standard Pick built-in functions in BAS
//
// ---- Includes ----
```

## 1. Description & Usage

Serializes a Set or Group structure into a Pick MultiValue dynamic array string.

## 2. Syntax

```basic
DYNARRAY$(set_or_group)
```

## 3. Code Example

```basic
10 Val = DYNARRAY$(set_or_group)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Pick Dynamic Arrays
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DYNARRAY$ |
| Category | Pick Dynamic Arrays |
| Syntax | DYNARRAY$(set_or_group) |
| Description | Serializes a Set or Group structure into a Pick MultiValue dynamic array string. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/pick.c |
