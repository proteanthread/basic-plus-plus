<!--
Title:        MCASE$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/runtime/string/strops.c
Generated:    no, hand-written
Status:       current
-->

# `MCASE$` Keyword Reference

## Source Header

```c
// FILENAME: strops.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libhardware, libkernel, libserver, 
// NEEDS: libcore (ctype.h, ctype.c, memops.h, memops.c, strops.h)
// Freestanding string search and manipulation implementation.
//
// ---- Includes ----
```

## 1. Description & Usage

Converts string str$ to mixed/title case with first letter of each word capitalized.

## 2. Syntax

```basic
MCASE$(str$)
```

## 3. Code Example

```basic
10 Val = MCASE$(str$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 14: Out of String Space

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: String Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | MCASE$ |
| Category | String Functions |
| Syntax | MCASE$(str$) |
| Description | Converts string str$ to mixed/title case with first letter of each word capitalized. |
| Error Summary | Error 14: Out of String Space |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/runtime/string/strops.c |
