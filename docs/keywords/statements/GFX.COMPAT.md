<!--
Title:        GFX.COMPAT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/graphics/compat.c
Generated:    no, hand-written
Status:       current
-->

# `GFX.COMPAT` Keyword Reference

## Source Header

```c
// FILENAME: compat.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (compat.h, eval.h, eval.c, string.c, vm.h)
// NEEDS: libkernel (bgi_gfx.h, bgi_gfx.c, errors.h)
// Provides runtime implementation for the COMPAT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Sets retro graphics hardware compatibility profiles (CGA, EGA, VGA, Atari, Hercules).

## 2. Syntax

```basic
SET MODE mode_num
```

## 3. Code Example

```basic
10 REM GFX.COMPAT Demonstration
20 PRINT "GFX.COMPAT executed successfully."
```

## 4. Error Conditions

Error 5: Illegal Function Call (unsupported graphics mode)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Graphics & Retro Modes
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | GFX.COMPAT |
| Category | Graphics & Retro Modes |
| Syntax | SET MODE mode_num |
| Description | Sets retro graphics hardware compatibility profiles (CGA, EGA, VGA, Atari, Hercules). |
| Error Summary | Error 5: Illegal Function Call (unsupported graphics mode) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/graphics/compat.c |
