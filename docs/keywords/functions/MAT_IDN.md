<!--
Title:        MAT_IDN
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/module/mathext.c
Generated:    no, hand-written
Status:       current
-->

# `MAT_IDN` Keyword Reference

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

Generates an identity matrix of specified dimensions with ones on diagonal and zeros elsewhere.

## 2. Syntax

```basic
MAT_IDN(rows [, cols])
```

## 3. Code Example

```basic
10 Val = MAT_IDN(rows [, cols])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Matrix Operations
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | MAT_IDN |
| Category | Matrix Operations |
| Syntax | MAT_IDN(rows [, cols]) |
| Description | Generates an identity matrix of specified dimensions with ones on diagonal and zeros elsewhere. |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/module/mathext.c |
