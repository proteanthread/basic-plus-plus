<!--
Title:        MKSMBF$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/conversion/mksmbf.c
Generated:    no, hand-written
Status:       current
-->

# `MKSMBF$` Keyword Reference

## Source Header

```c
// FILENAME: mksmbf.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, conversion_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (mbf_common.h, mksmbf.h)
// Provides runtime implementation for the MKSMBF$ function in BASIC++.
```

## 1. Description & Usage

Converts single-precision float to 4-byte Microsoft Binary Format (MBF) string.

## 2. Syntax

```basic
s$ = MKSMBF$(x!)
```

## 3. Code Example

```basic
10 Val = s$ = MKSMBF$(10!)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Type Conversion
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | MKSMBF$ |
| Category | Type Conversion |
| Syntax | s$ = MKSMBF$(x!) |
| Description | Converts single-precision float to 4-byte Microsoft Binary Format (MBF) string. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/conversion/mksmbf.c |
