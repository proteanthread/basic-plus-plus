<!--
Title:        HOME
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/graphics/stmt_home.c
Generated:    no, hand-written
Status:       current
-->

# `HOME` Keyword Reference

## Source Header

```c
// FILENAME: stmt_home.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (color_map.h, eval.h, lexer.h, stmt_home.h, vm.h)
// NEEDS: libkernel (errors.h, vcon.h, vdev.h)
// Provides runtime implementation for the HOME statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Moves cursor to top-left (1, 1) without clearing screen, optionally setting text colors.

## 2. Syntax

```basic
HOME [[fg] [, bg]]
```

## 3. Code Example

```basic
10 REM HOME Demonstration
20 PRINT "HOME executed successfully."
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
| Name | HOME |
| Category | Graphics & Display |
| Syntax | HOME [[fg] [, bg]] |
| Description | Moves cursor to top-left (1, 1) without clearing screen, optionally setting text colors. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/graphics/stmt_home.c |
