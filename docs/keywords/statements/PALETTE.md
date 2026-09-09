<!--
Title:        PALETTE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/graphics/screen/palette.c
Generated:    no, hand-written
Status:       current
-->

# `PALETTE` Keyword Reference

## Source Header

```c
// FILENAME: palette.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, palette.h, string.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the PALETTE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Changes color mapping for screen palette attributes.

## 2. Syntax

```basic
PALETTE [attribute, color]
```

## 3. Code Example

```basic
10 REM PALETTE Demonstration
20 PRINT "PALETTE executed successfully."
```

## 4. Error Conditions

Error 5: Illegal Function Call (invalid attribute or color)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Graphics & Color
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | PALETTE |
| Category | Graphics & Color |
| Syntax | PALETTE [attribute, color] |
| Description | Changes color mapping for screen palette attributes. |
| Error Summary | Error 5: Illegal Function Call (invalid attribute or color) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/graphics/screen/palette.c |
