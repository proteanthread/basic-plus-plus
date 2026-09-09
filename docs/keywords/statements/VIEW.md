<!--
Title:        VIEW
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/graphics/screen/view.c
Generated:    no, hand-written
Status:       current
-->

# `VIEW` Keyword Reference

## Source Header

```c
// FILENAME: view.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, view.h, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the VIEW statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Defines screen viewport boundaries for graphics clipping and drawing.

## 2. Syntax

```basic
VIEW [[SCREEN] (x1, y1)-(x2, y2) [, fill_color [, border_color]]]
```

## 3. Code Example

```basic
10 Val = VIEW [[SCREEN] (101, y1)-(102, y2) [, fill_color [, border_color]]]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call (invalid viewport bounds)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Graphics & Viewports
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | VIEW |
| Category | Graphics & Viewports |
| Syntax | VIEW [[SCREEN] (x1, y1)-(x2, y2) [, fill_color [, border_color]]] |
| Description | Defines screen viewport boundaries for graphics clipping and drawing. |
| Error Summary | Error 5: Illegal Function Call (invalid viewport bounds) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/graphics/screen/view.c |
