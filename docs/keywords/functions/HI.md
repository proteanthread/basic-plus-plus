<!--
Title:        HI
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/eval.c
Generated:    no, hand-written
Status:       current
-->

# `HI` Keyword Reference

## Source Header

```c
// FILENAME: eval.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libext, libkernel, libstandard
// NEEDS: libengine (eval_expr_internal.h)
// Implements component functionality for eval.c.
//
// ---- Includes ----
```

## 1. Description & Usage

Extracts the high-order byte or word of numeric expression val%.

## 2. Syntax

```basic
HI(val%)
```

## 3. Code Example

```basic
10 Val = HI(val%)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Bitwise & Binary
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | HI |
| Category | Bitwise & Binary |
| Syntax | HI(val%) |
| Description | Extracts the high-order byte or word of numeric expression val%. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/eval/eval.c |
