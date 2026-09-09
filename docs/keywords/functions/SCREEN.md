<!--
Title:        SCREEN
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/graphics/screen/screen.c
Generated:    no, hand-written
Status:       current
-->

# `SCREEN` Keyword Reference

## Source Header

```c
// FILENAME: screen.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, screen.h, string.c, vm.
// NEEDS: libkernel (bgi_autodetect.h, bgi_autodetect.c, bgi_gfx.h, bgi_gfx.c)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the SCREEN statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Sets text (SCREEN 0), heritage (SCREEN 1-13), or modern high-res display mode (SCREEN 14-20: 800x600 to 4K).

## 2. Syntax

```basic
SCREEN mode [, [colorswitch] [, [active_page] [, visual_page]]]
```

## 3. Code Example

```basic
10 REM SCREEN Demonstration
20 PRINT "SCREEN executed successfully."
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
| Name | SCREEN |
| Category | Graphics & Display |
| Syntax | SCREEN mode [, [colorswitch] [, [active_page] [, visual_page]]] |
| Description | Sets text (SCREEN 0), heritage (SCREEN 1-13), or modern high-res display mode (SCREEN 14-20: 800x600 to 4K). |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/graphics/screen/screen.c |
