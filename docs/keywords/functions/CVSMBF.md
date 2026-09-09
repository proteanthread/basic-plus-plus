<!--
Title:        CVSMBF
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/conversion/cvsmbf.c
Generated:    no, hand-written
Status:       current
-->

# `CVSMBF` Keyword Reference

## Source Header

```c
// FILENAME: cvsmbf.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, conversion_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (cvsmbf.h, mbf_common.h)
// Provides runtime implementation for the CVSMBF function in BASIC++.
```

## 1. Description & Usage

Converts 4-byte Microsoft Binary Format (MBF) string to single-precision float.

## 2. Syntax

```basic
x! = CVSMBF(s$)
```

## 3. Code Example

```basic
10 Val = 10! = CVSMBF(s$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call, Error 13: Type Mismatch

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
| Name | CVSMBF |
| Category | Type Conversion |
| Syntax | x! = CVSMBF(s$) |
| Description | Converts 4-byte Microsoft Binary Format (MBF) string to single-precision float. |
| Error Summary | Error 5: Illegal Function Call, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/conversion/cvsmbf.c |
