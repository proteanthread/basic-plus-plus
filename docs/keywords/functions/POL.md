<!--
Title:        POL
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/trig/pol_rec.c
Generated:    no, hand-written
Status:       current
-->

# `POL` Keyword Reference

## Source Header

```c
// FILENAME: pol_rec.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c, dispatch_call.c)
// NEEDS: libkernel (vm.h, eval.h)
// Provides runtime implementation for POL and REC coordinate conversion funct
//
// ---- Includes ----
```

## 1. Description & Usage

Converts rectangular coordinates (x, y) to polar coordinates (r, theta) (Sharp Pocket BASIC PC-1211/PC-1500 / Casio).

## 2. Syntax

```basic
POL(x, y [, idx]) | POL[x, y]
```

## 3. Code Example

```basic
10 Val = POL(10, y [, id10]) | POL[10, y]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Math & Trigonometry
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | POL |
| Category | Math & Trigonometry |
| Syntax | POL(x, y [, idx]) \| POL[x, y] |
| Description | Converts rectangular coordinates (x, y) to polar coordinates (r, theta) (Sharp Pocket BASIC PC-1211/PC-1500 / Casio). |
| Error Summary | Error 13: Type Mismatch, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/math/trig/pol_rec.c |
