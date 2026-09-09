<!--
Title:        CLS
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/graphics/screen/cls.c
Generated:    no, hand-written
Status:       current
-->

# `CLS` Keyword Reference

## Source Header

```c
// FILENAME: cls.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (cls.h, color_map.h, eval.h, lexer.h, vm.h)
// NEEDS: libkernel (errors.h, vcon.h, vdev.h)
// Provides runtime implementation for the CLS statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Clears screen and moves cursor to (1, 1), optionally setting foreground and background colors.

## 2. Syntax

```basic
CLS [[fg] [, bg]]
```

## 3. Code Example

```basic
10 SCREEN 0 : WIDTH 80
20 COLOR 14, 1
30 CLS
40 PRINT "Screen cleared."
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
| Name | CLS |
| Category | Graphics & Display |
| Syntax | CLS [[fg] [, bg]] |
| Description | Clears screen and moves cursor to (1, 1), optionally setting foreground and background colors. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/graphics/screen/cls.c |
