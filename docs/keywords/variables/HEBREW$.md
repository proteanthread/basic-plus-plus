<!--
Title:        HEBREW$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/datetime/hebrew.c
Generated:    no, hand-written
Status:       current
-->

# `HEBREW$` Keyword Reference

## Source Header

```c
// FILENAME: hebrew.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, eval_ident_builtin.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (hebrew.h)
// Provides runtime implementation and LanguageDescriptor for HEBREW$ system v
```

## 1. Description & Usage

Returns today's Hebrew calendar date formatted as a string (e.g. '19 Elul 5786').

## 2. Syntax

```basic
HEBREW$
```

## 3. Code Example

```basic
10 REM HEBREW$ Demonstration
20 PRINT "HEBREW$ executed successfully."
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
| Name | HEBREW$ |
| Category | Clocks & Timers |
| Syntax | HEBREW$ |
| Description | Returns today's Hebrew calendar date formatted as a string (e.g. '19 Elul 5786'). |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_VARIABLE |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/datetime/hebrew.c |
