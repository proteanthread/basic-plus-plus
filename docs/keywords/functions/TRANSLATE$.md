<!--
Title:        TRANSLATE$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/translate.c
Generated:    no, hand-written
Status:       current
-->

# `TRANSLATE$` Keyword Reference

## Source Header

```c
// FILENAME: translate.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c, stmt_translate.c)
// NEEDS: libkernel (vm.h, eval.h, strings.h)
// Provides runtime implementation for TRANSLATE$ character mapping in BASIC++
//
// ---- Includes ----
```

## 1. Description & Usage

Maps and translates characters in a string according to translation table or pair mappings (Wang 2200).

## 2. Syntax

```basic
TRANSLATE$(src$, from$, to$) | TRANSLATE$(src$, table$) | TRANSLATE$[from$, to$](src$)
```

## 3. Code Example

```basic
10 Val = TRANSLATE$(src$, from$, to$) | TRANSLATE$(src$, table$) | TRANSLATE$[from$, to$](src$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: String Manipulation
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | TRANSLATE$ |
| Category | String Manipulation |
| Syntax | TRANSLATE$(src$, from$, to$) \| TRANSLATE$(src$, table$) \| TRANSLATE$[from$, to$](src$) |
| Description | Maps and translates characters in a string according to translation table or pair mappings (Wang 2200). |
| Error Summary | Error 13: Type Mismatch, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/translate.c |
