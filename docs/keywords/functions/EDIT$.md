<!--
Title:        EDIT$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/runtime/string/strops.c
Generated:    no, hand-written
Status:       current
-->

# `EDIT$` Keyword Reference

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

Transforms string str$ according to bitmask actions (trim, compress spaces, lowercase, etc.).

## 2. Syntax

```basic
EDIT$(str$, action_mask%)
```

## 3. Code Example

```basic
10 Val = EDIT$(str$, action_mask%)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call

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
| Name | EDIT$ |
| Category | String Functions |
| Syntax | EDIT$(str$, action_mask%) |
| Description | Transforms string str$ according to bitmask actions (trim, compress spaces, lowercase, etc.). |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/runtime/string/strops.c |
