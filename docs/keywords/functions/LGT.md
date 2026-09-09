<!--
Title:        LGT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/runtime/math/math.c
Generated:    no, hand-written
Status:       current
-->

# `LGT` Keyword Reference

## Source Header

```c
// FILENAME: math.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for LGT.
```

## 1. Description & Usage

Computes the common logarithm (base-10 logarithm) of positive number x.

## 2. Syntax

```basic
LGT(x)
```

## 3. Code Example

```basic
10 Val = LGT(10)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call (x <= 0)

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
| Name | LGT |
| Category | Math Functions |
| Syntax | LGT(x) |
| Description | Computes the common logarithm (base-10 logarithm) of positive number x. |
| Error Summary | Error 5: Illegal Function Call (x <= 0) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/runtime/math/math.c |
