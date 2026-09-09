<!--
Title:        POINT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/ui/graphics/point_fn.c
Generated:    no, hand-written
Status:       current
-->

# `POINT` Keyword Reference

## Source Header

```c
// FILENAME: point_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (point_fn.h, vm.h)
// NEEDS: libkernel (vcon.h, vcon.c)
// Provides runtime implementation for the POINT_FN built-in function in BASIC
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the color of the pixel at (x, y) or the current graphics coordinate.

## 2. Syntax

```basic
color% = POINT(x%, y%) | coord% = POINT(mode%)
```

## 3. Code Example

```basic
10 Val = color% = POINT(10%, y%) | coord% = POINT(mode%)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (POINT expects 1 or 2 numeric arguments)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Graphics Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | POINT |
| Category | Graphics Functions |
| Syntax | color% = POINT(x%, y%) \| coord% = POINT(mode%) |
| Description | Returns the color of the pixel at (x, y) or the current graphics coordinate. |
| Error Summary | Error 13: Type Mismatch (POINT expects 1 or 2 numeric arguments) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/ui/graphics/point_fn.c |
