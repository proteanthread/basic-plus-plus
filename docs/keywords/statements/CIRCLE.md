<!--
Title:        CIRCLE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/graphics/draw/circle.c
Generated:    no, hand-written
Status:       current
-->

# `CIRCLE` Keyword Reference

## Source Header

```c
// FILENAME: circle.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (circle.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the CIRCLE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Draws a circle, ellipse, or arc on the active graphics screen canvas.

## 2. Syntax

```basic
CIRCLE (x, y), radius [, [color] [, [start] [, [end] [, aspect]]]]
```

## 3. Code Example

```basic
10 Val = CIRCLE (10, y), radius [, [color] [, [start] [, [end] [, aspect]]]]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Graphics & Display
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CIRCLE |
| Category | Graphics & Display |
| Syntax | CIRCLE (x, y), radius [, [color] [, [start] [, [end] [, aspect]]]] |
| Description | Draws a circle, ellipse, or arc on the active graphics screen canvas. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/graphics/draw/circle.c |
