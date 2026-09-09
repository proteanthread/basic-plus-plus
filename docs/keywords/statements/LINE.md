<!--
Title:        LINE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/graphics/draw/line.c
Generated:    no, hand-written
Status:       current
-->

# `LINE` Keyword Reference

## Source Header

```c
// FILENAME: line.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, input_file.h, input_file.c)
// NEEDS: libengine (lexer.h, lexer.c, line.h, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the LINE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Draws a straight line or box on the active graphics screen canvas.

## 2. Syntax

```basic
LINE [[x1, y1]]-(x2, y2) [, [color] [, [B|BF] [, style]]]
```

## 3. Code Example

```basic
10 Val = LINE [[101, y1]]-(102, y2) [, [color] [, [B|BF] [, style]]]
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
| Name | LINE |
| Category | Graphics & Display |
| Syntax | LINE [[x1, y1]]-(x2, y2) [, [color] [, [B\|BF] [, style]]] |
| Description | Draws a straight line or box on the active graphics screen canvas. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/graphics/draw/line.c |
