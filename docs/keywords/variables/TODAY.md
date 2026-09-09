<!--
Title:        TODAY
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/environment/func_ver.c
Generated:    no, hand-written
Status:       current
-->

# `TODAY` Keyword Reference

## Source Header

```c
// FILENAME: func_ver.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, eval_ident_builtin.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (func_ver.h)
// Provides runtime implementation and LanguageDescriptors for version, errorl
```

## 1. Description & Usage

Returns current date as a pure numeric integer YYYYMMDD.

## 2. Syntax

```basic
TODAY
```

## 3. Code Example

```basic
10 REM TODAY Demonstration
20 PRINT "TODAY executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Clocks & Timers
- **Subsystem**: SUBSYSTEM_PLATFORM
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | TODAY |
| Category | Clocks & Timers |
| Syntax | TODAY |
| Description | Returns current date as a pure numeric integer YYYYMMDD. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_VARIABLE |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/environment/func_ver.c |
