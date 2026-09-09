<!--
Title:        CVT$%
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/conversion/cvt.c
Generated:    no, hand-written
Status:       current
-->

# `CVT$%` Keyword Reference

## Source Header

```c
// FILENAME: cvt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c, string_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (cvt.h, string.c)
// Provides runtime implementation for the CVT built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Performs DEC PDP-11 / BASIC-PLUS binary word/byte conversion, float packing, character transformations, and byte swapping.

## 2. Syntax

```basic
CVT$%(str_val) | CVT%$(int_val) | CVT$F(str_val) | CVTF$(flt_val) | CVT$$(str_val, flags) | SWAP%(int_val)
```

## 3. Code Example

```basic
10 Val = CVT$%(str_val) | CVT%$(int_val) | CVT$F(str_val) | CVTF$(flt_val) | CVT$$(str_val, flags) | SWAP%(int_val)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call, Error 13: Type Mismatch

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
| Name | CVT$% |
| Category | String Functions |
| Syntax | CVT$%(str_val) \| CVT%$(int_val) \| CVT$F(str_val) \| CVTF$(flt_val) \| CVT$$(str_val, flags) \| SWAP%(int_val) |
| Description | Performs DEC PDP-11 / BASIC-PLUS binary word/byte conversion, float packing, character transformations, and byte swapping. |
| Error Summary | Error 5: Illegal Function Call, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/conversion/cvt.c |
