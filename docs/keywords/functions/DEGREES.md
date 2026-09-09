<!--
Title:        DEGREES
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/trig/degrees.c
Generated:    no, hand-written
Status:       current
-->

# `DEGREES` Keyword Reference

## Source Header

```c
// FILENAME: degrees.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (degrees.h, math.c, string.c)
// Provides runtime implementation for the DEGREES built-in function in BASIC+
//
// ---- Includes ----
```

## 1. Description & Usage

Converts angle x from radians to degrees (ANSI Full BASIC 1987).

## 2. Syntax

```basic
DEGREES(x)
```

## 3. Code Example

```basic
10 Val = DEGREES(10)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (DEGREES expects one numeric argument)

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
| Name | DEGREES |
| Category | Math Functions |
| Syntax | DEGREES(x) |
| Description | Converts angle x from radians to degrees (ANSI Full BASIC 1987). |
| Error Summary | Error 13: Type Mismatch (DEGREES expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/trig/degrees.c |
