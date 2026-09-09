<!--
Title:        PLOT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/graphics/draw/stmt_plot.c
Generated:    no, hand-written
Status:       current
-->

# `PLOT` Keyword Reference

## Source Header

```c
// FILENAME: stmt_plot.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, stmt_plot.h, string.c)
// NEEDS: libengine (vm.h)
// Provides runtime implementation for the PLOT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Plots points, lines, or filled polygon area (ANSI Full BASIC 1987).

## 2. Syntax

```basic
PLOT [POINTS|LINES|AREA]: x, y [; x2, y2 ...]
```

## 3. Code Example

```basic
10 REM PLOT Demonstration
20 PRINT "PLOT executed successfully."
```

## 4. Error Conditions

Error 5: Illegal Function Call, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Graphics Statements
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | PLOT |
| Category | Graphics Statements |
| Syntax | PLOT [POINTS\|LINES\|AREA]: x, y [; x2, y2 ...] |
| Description | Plots points, lines, or filled polygon area (ANSI Full BASIC 1987). |
| Error Summary | Error 5: Illegal Function Call, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/graphics/draw/stmt_plot.c |
