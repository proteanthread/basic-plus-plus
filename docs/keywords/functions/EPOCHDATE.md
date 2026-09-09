<!--
Title:        EPOCHDATE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/datetime/epochdate.c
Generated:    no, hand-written
Status:       current
-->

# `EPOCHDATE` Keyword Reference

## Source Header

```c
// FILENAME: epochdate.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (hal.h, language_descriptor.h)
// NEEDS: libengine (epochdate.h)
// Provides runtime implementation for the EPOCHDATE function in BASIC++.
```

## 1. Description & Usage

Converts Microsoft Serial Date to Unix epoch seconds or returns current timestamp.

## 2. Syntax

```basic
EPOCHDATE([serial#])
```

## 3. Code Example

```basic
10 Val = EPOCHDATE([serial#])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Date and Time
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | EPOCHDATE |
| Category | Date and Time |
| Syntax | EPOCHDATE([serial#]) |
| Description | Converts Microsoft Serial Date to Unix epoch seconds or returns current timestamp. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/datetime/epochdate.c |
