<!--
Title:        DRAW
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/graphics/draw/draw.c
Generated:    no, hand-written
Status:       current
-->

# `DRAW` Keyword Reference

## Source Header

```c
// FILENAME: draw.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (bgi.h, bgi.c, draw.h, eval.h, eval.c, lexer.h, lexer.c)
// NEEDS: libengine (string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the DRAW statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Executes graphics macro commands to draw shapes using vector movement language.

## 2. Syntax

```basic
DRAW command_str$
```

## 3. Code Example

```basic
10 REM DRAW Demonstration
20 PRINT "DRAW executed successfully."
```

## 4. Error Conditions

Error 5: Illegal Function Call (invalid DRAW command string)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Graphics & Drawing
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DRAW |
| Category | Graphics & Drawing |
| Syntax | DRAW command_str$ |
| Description | Executes graphics macro commands to draw shapes using vector movement language. |
| Error Summary | Error 5: Illegal Function Call (invalid DRAW command string) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/graphics/draw/draw.c |
