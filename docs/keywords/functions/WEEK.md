<!--
Title:        WEEK
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/datetime/func_week.c
Generated:    no, hand-written
Status:       current
-->

# `WEEK` Keyword Reference

## Source Header

```c
// FILENAME: func_week.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_funcs.c, eval_ident_builtin.c
// NEEDS: libkernel, libcore, libplatform
// Implementation for WEEK and WEEK$ date functions in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns current week number of the year (ww) as a numeric integer.

## 2. Syntax

```basic
WEEK or WEEK()
```

## 3. Code Example

```basic
10 Val = WEEK or WEEK()
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Clocks & Timers
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | WEEK |
| Category | Clocks & Timers |
| Syntax | WEEK or WEEK() |
| Description | Returns current week number of the year (ww) as a numeric integer. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/datetime/func_week.c |
