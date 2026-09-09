<!--
Title:        AT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/core/io/print.c
Generated:    no, hand-written
Status:       current
-->

# `AT` Keyword Reference

## Source Header

```c
// FILENAME: print.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (dialect.h, dialect.c, math.h)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libcore (num_format.h, num_format.c, string.h, using.h)
// NEEDS: libengine (eval.h, eval.c, math.c, stmt.h, string.c)
// NEEDS: libkernel (vcon.h, vcon.c, vdev.h, vdev.c)
// Provides runtime implementation for the PRINT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Screen cursor positioning clause for 2D Cartesian grid, graphics pixel offload, and declarative property maps.

## 2. Syntax

```basic
PRINT AT x, y [, fg [, bg]] | AT(x, y [, fg [, bg]]) | AT[px, py] | AT{prop: val, ...}
```

## 3. Code Example

```basic
10 Val = PRINT AT 10, y [, fg [, bg]] | AT(10, y [, fg [, bg]]) | AT[p10, py] | AT{prop: val, ...}
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Console I/O
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | AT |
| Category | Console I/O |
| Syntax | PRINT AT x, y [, fg [, bg]] \| AT(x, y [, fg [, bg]]) \| AT[px, py] \| AT{prop: val, ...} |
| Description | Screen cursor positioning clause for 2D Cartesian grid, graphics pixel offload, and declarative property maps. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/core/io/print.c |
