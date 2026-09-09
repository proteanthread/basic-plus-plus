<!--
Title:        HTA$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/conversion/hta.c
Generated:    no, hand-written
Status:       current
-->

# `HTA$` Keyword Reference

## Source Header

```c
// FILENAME: hta.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, string_fn.c)
// NEEDS: libcore (hal.h, language_descriptor.h, strings.h)
// NEEDS: libengine (hta.h)
// Provides runtime implementation for the HTA$ function in BASIC++.
```

## 1. Description & Usage

Converts ASCII characters or numeric values to Hex string representation (Basic Four / BBx).

## 2. Syntax

```basic
HTA$(str$ | num)
```

## 3. Code Example

```basic
10 Val = HTA$(str$ | num)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: String Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | HTA$ |
| Category | String Functions |
| Syntax | HTA$(str$ \| num) |
| Description | Converts ASCII characters or numeric values to Hex string representation (Basic Four / BBx). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/conversion/hta.c |
