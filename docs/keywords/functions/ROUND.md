<!--
Title:        ROUND
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/algebra/round.c
Generated:    no, hand-written
Status:       current
-->

# `ROUND` Keyword Reference

## Source Header

```c
// FILENAME: runtime_round.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, runtime_round.h, string.c)
// Provides runtime implementation for the ROUND built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Rounds a numeric expression x to the specified number of decimal places (default 0).

## 2. Syntax

```basic
ROUND(x [, decimals])
```

## 3. Code Example

```basic
10 Val = ROUND(10 [, decimals])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (ROUND expects 1 or 2 numeric arguments)

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
| Name | ROUND |
| Category | Math Functions |
| Syntax | ROUND(x [, decimals]) |
| Description | Rounds a numeric expression x to the specified number of decimal places (default 0). |
| Error Summary | Error 13: Type Mismatch (ROUND expects 1 or 2 numeric arguments) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/algebra/round.c |
