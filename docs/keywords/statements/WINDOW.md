<!--
Title:        WINDOW
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/graphics/screen/window.c
Generated:    no, hand-written
Status:       current
-->

# `WINDOW` Keyword Reference

## Source Header

```c
// FILENAME: window.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h, window.
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the WINDOW statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Defines world coordinate mapping for graphics viewport transformation.

## 2. Syntax

```basic
WINDOW [[SCREEN] (x1, y1)-(x2, y2)]
```

## 3. Code Example

```basic
10 Val = WINDOW [[SCREEN] (101, y1)-(102, y2)]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call (coordinates out of bounds)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Graphics & Coordinates
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | WINDOW |
| Category | Graphics & Coordinates |
| Syntax | WINDOW [[SCREEN] (x1, y1)-(x2, y2)] |
| Description | Defines world coordinate mapping for graphics viewport transformation. |
| Error Summary | Error 5: Illegal Function Call (coordinates out of bounds) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/graphics/screen/window.c |
