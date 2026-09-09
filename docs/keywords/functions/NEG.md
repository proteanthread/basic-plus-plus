<!--
Title:        NEG
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/arithmetic/func_neg.c
Generated:    no, hand-written
Status:       current
-->

# `NEG` Keyword Reference

## Source Header

```c
// FILENAME: func_neg.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_funcs.c, eval_ident.c, ast_parse_exp
// NEEDS: libkernel, libcore
// Implementation for arithmetic negation function (NEG) in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the arithmetic negation (-x) of a numeric expression.

## 2. Syntax

```basic
NEG(x) or NEG x
```

## 3. Code Example

```basic
10 Val = NEG(10) or NEG 10
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Math
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | NEG |
| Category | Math |
| Syntax | NEG(x) or NEG x |
| Description | Returns the arithmetic negation (-x) of a numeric expression. |
| Error Summary | Error 13: Type mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/arithmetic/func_neg.c |
