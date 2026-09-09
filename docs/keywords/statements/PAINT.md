<!--
Title:        PAINT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/graphics/draw/paint.c
Generated:    no, hand-written
Status:       current
-->

# `PAINT` Keyword Reference

## Source Header

```c
// FILENAME: paint.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, paint.h, string.c, vm.h
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the PAINT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Fills a closed graphics region starting at coordinates (x, y) with color or tile pattern.

## 2. Syntax

```basic
PAINT (x, y) [, fill_color [, border_color]]
```

## 3. Code Example

```basic
10 Val = PAINT (10, y) [, fill_color [, border_color]]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call (coordinates out of bounds)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Graphics & Drawing
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | PAINT |
| Category | Graphics & Drawing |
| Syntax | PAINT (x, y) [, fill_color [, border_color]] |
| Description | Fills a closed graphics region starting at coordinates (x, y) with color or tile pattern. |
| Error Summary | Error 5: Illegal Function Call (coordinates out of bounds) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/graphics/draw/paint.c |
