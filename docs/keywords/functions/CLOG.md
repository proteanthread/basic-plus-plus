<!--
Title:        CLOG
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/module/mathext.c
Generated:    no, hand-written
Status:       current
-->

# `CLOG` Keyword Reference

## Source Header

```c
// FILENAME: mathext.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, funcreg.c, math.h)
// NEEDS: libengine (math.c)
// Provides core logic and interface definitions for mathext within BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Computes the complex natural logarithm ln(z) of complex number z.

## 2. Syntax

```basic
CLOG(z)
```

## 3. Code Example

```basic
10 Val = CLOG(z)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call (z=0), Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Math & Complex Numbers
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CLOG |
| Category | Math & Complex Numbers |
| Syntax | CLOG(z) |
| Description | Computes the complex natural logarithm ln(z) of complex number z. |
| Error Summary | Error 5: Illegal Function Call (z=0), Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/module/mathext.c |
