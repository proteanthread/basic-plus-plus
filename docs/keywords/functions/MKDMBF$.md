<!--
Title:        MKDMBF$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/conversion/mkdmbf.c
Generated:    no, hand-written
Status:       current
-->

# `MKDMBF$` Keyword Reference

## Source Header

```c
// FILENAME: mkdmbf.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, conversion_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (mbf_common.h, mkdmbf.h)
// Provides runtime implementation for the MKDMBF$ function in BASIC++.
```

## 1. Description & Usage

Converts double-precision float to 8-byte Microsoft Binary Format (MBF) string.

## 2. Syntax

```basic
s$ = MKDMBF$(x#)
```

## 3. Code Example

```basic
10 Val = s$ = MKDMBF$(10#)
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
| Name | MKDMBF$ |
| Category | Type Conversion |
| Syntax | s$ = MKDMBF$(x#) |
| Description | Converts double-precision float to 8-byte Microsoft Binary Format (MBF) string. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/conversion/mkdmbf.c |
