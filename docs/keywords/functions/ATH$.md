<!--
Title:        ATH$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/conversion/ath.c
Generated:    no, hand-written
Status:       current
-->

# `ATH$` Keyword Reference

## Source Header

```c
// FILENAME: ath.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, string_fn.c)
// NEEDS: libcore (hal.h, language_descriptor.h, strings.h)
// NEEDS: libengine (ath.h)
// Provides runtime implementation for the ATH$ function in BASIC++.
```

## 1. Description & Usage

Converts Hex string representation to raw ASCII byte string (Basic Four / BBx).

## 2. Syntax

```basic
ATH$(hex_str$)
```

## 3. Code Example

```basic
10 Val = ATH$(he10_str$)
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
| Name | ATH$ |
| Category | String Functions |
| Syntax | ATH$(hex_str$) |
| Description | Converts Hex string representation to raw ASCII byte string (Basic Four / BBx). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/conversion/ath.c |
