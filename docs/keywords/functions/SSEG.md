<!--
Title:        SSEG
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/environment/sseg.c
Generated:    no, hand-written
Status:       current
-->

# `SSEG` Keyword Reference

## Source Header

```c
// FILENAME: sseg.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (sseg.h)
// Provides runtime implementation for the SSEG function in BASIC++.
```

## 1. Description & Usage

Returns the segment address of the string descriptor.

## 2. Syntax

```basic
SSEG(var)
```

## 3. Code Example

```basic
10 Val = SSEG(var)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | SSEG |
| Category | System Functions |
| Syntax | SSEG(var) |
| Description | Returns the segment address of the string descriptor. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/environment/sseg.c |
