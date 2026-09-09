<!--
Title:        COLOR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/graphics/screen/color.c
Generated:    no, hand-written
Status:       current
-->

# `COLOR` Keyword Reference

## Source Header

```c
// FILENAME: color.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (color.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h
// NEEDS: libkernel (vcon.h, vcon.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the COLOR statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Sets active foreground, background, and border display colors.

## 2. Syntax

```basic
COLOR [foreground] [, [background] [, border]]
```

## 3. Code Example

```basic
10 REM COLOR Demonstration
20 PRINT "COLOR executed successfully."
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
| Name | COLOR |
| Category | Graphics & Display |
| Syntax | COLOR [foreground] [, [background] [, border]] |
| Description | Sets active foreground, background, and border display colors. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/graphics/screen/color.c |
