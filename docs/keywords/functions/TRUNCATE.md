<!--
Title:        TRUNCATE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/algebra/truncate.c
Generated:    no, hand-written
Status:       current
-->

# `TRUNCATE` Keyword Reference

## Source Header

```c
// FILENAME: truncate.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, string.c, truncate.h)
// Provides runtime implementation for the TRUNCATE built-in function in BASIC
//
// ---- Includes ----
```

## 1. Description & Usage

Truncates numeric x to n decimal places toward zero (ANSI Full BASIC 1987).

## 2. Syntax

```basic
TRUNCATE(x [, n])
```

## 3. Code Example

```basic
10 Val = TRUNCATE(10 [, n])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Math Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | TRUNCATE |
| Category | Math Functions |
| Syntax | TRUNCATE(x [, n]) |
| Description | Truncates numeric x to n decimal places toward zero (ANSI Full BASIC 1987). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/algebra/truncate.c |
