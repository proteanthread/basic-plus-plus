<!--
Title:        BGI
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/graphics/bgi.c
Generated:    no, hand-written
Status:       current
-->

# `BGI` Keyword Reference

## Source Header

```c
// FILENAME: bgi.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libhardware, libkernel
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (bgi.h, eval.h, eval.c, stmt.h, string.c, vm.h)
// Provides runtime implementation for the BGI statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Configures custom screen resolution and BGI graphics mode settings.

## 2. Syntax

```basic
SET GRAPHICS width, height [, bpp]
```

## 3. Code Example

```basic
10 REM BGI Demonstration
20 PRINT "BGI executed successfully."
```

## 4. Error Conditions

Error 5: Illegal Function Call (invalid graphics dimensions)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Graphics Interface
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | BGI |
| Category | Graphics Interface |
| Syntax | SET GRAPHICS width, height [, bpp] |
| Description | Configures custom screen resolution and BGI graphics mode settings. |
| Error Summary | Error 5: Illegal Function Call (invalid graphics dimensions) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/graphics/bgi.c |
