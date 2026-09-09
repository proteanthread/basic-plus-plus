<!--
Title:        ASC
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/builtins/string_fn.c
Generated:    no, hand-written
Status:       current
-->

# `ASC` Keyword Reference

## Source Header

```c
// FILENAME: string_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine
// Provides core logic and interface definitions for string_fn within BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the numeric ASCII / byte value (0 to 255) of the first character of string_expr$.

## 2. Syntax

```basic
ASC(string_expr$)
```

## 3. Code Example

```basic
10 Val = ASC(string_e10pr$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call (empty string)

## 5. Compatibility & Lineage

- **Lineage**: GW-BASIC, QBASIC, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: String Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | ASC |
| Category | String Functions |
| Syntax | ASC(string_expr$) |
| Description | Returns the numeric ASCII / byte value (0 to 255) of the first character of string_expr$. |
| Error Summary | Error 5: Illegal Function Call (empty string) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | GW-BASIC, QBASIC, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/eval/builtins/string_fn.c |
