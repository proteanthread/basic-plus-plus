<!--
Title:        ICASE$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/runtime/string/strops.c
Generated:    no, hand-written
Status:       current
-->

# `ICASE$` Keyword Reference

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

Inverts the case of each alphabetic character in string str$ (lower to upper, upper to lower).

## 2. Syntax

```basic
ICASE$(str$)
```

## 3. Code Example

```basic
10 Val = ICASE$(str$)
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
| Name | ICASE$ |
| Category | String Functions |
| Syntax | ICASE$(str$) |
| Description | Inverts the case of each alphabetic character in string str$ (lower to upper, upper to lower). |
| Error Summary | Error 14: Out of String Space |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/runtime/string/strops.c |
