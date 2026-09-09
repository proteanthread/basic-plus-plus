<!--
Title:        OVERLAY$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/func_overlay.c
Generated:    no, hand-written
Status:       current
-->

# `OVERLAY$` Keyword Reference

## Source Header

```c
// FILENAME: func_overlay.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libengine (func_overlay.h, vm.h)
// Provides runtime implementation for the OVERLAY$ string overwriting functio
```

## 1. Description & Usage

Overwrites characters in target$ with source$ starting at 1-based index pos (supports negative pos).

## 2. Syntax

```basic
OVERLAY$(target$, source$, pos)
```

## 3. Code Example

```basic
10 Val = OVERLAY$(target$, source$, pos)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch, Error 7: Out of Memory

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: String Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | OVERLAY$ |
| Category | String Functions |
| Syntax | OVERLAY$(target$, source$, pos) |
| Description | Overwrites characters in target$ with source$ starting at 1-based index pos (supports negative pos). |
| Error Summary | Error 13: Type Mismatch, Error 7: Out of Memory |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/func_overlay.c |
