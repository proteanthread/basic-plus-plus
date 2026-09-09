<!--
Title:        PRINT AT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/core/io/print_pos.c
Generated:    no, hand-written
Status:       current
-->

# `PRINT AT` Keyword Reference

## Source Header

```c
// FILENAME: print_pos.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (print.c)
// NEEDS: libcore (snprintf.h, strops.h, memops.h)
// NEEDS: libengine (eval.h, stmt.h, print_pos.h)
// Implements 2D grid and 1D buffer cursor positioning for PRINT AT and PRINT 
//
// ---- Includes ----
```

## 1. Description & Usage

Positions cursor at 2D grid coordinates or 1D buffer offset before printing.

## 2. Syntax

```basic
PRINT AT(x, y) [expr] | PRINT @pos [expr] | PRINT AT{x: col, y: row} [expr]
```

## 3. Code Example

```basic
10 Val = PRINT AT(10, y) [e10pr] | PRINT @pos [e10pr] | PRINT AT{10: col, y: row} [e10pr]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Console and Terminal I/O
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | PRINT AT |
| Category | Console and Terminal I/O |
| Syntax | PRINT AT(x, y) [expr] \| PRINT @pos [expr] \| PRINT AT{x: col, y: row} [expr] |
| Description | Positions cursor at 2D grid coordinates or 1D buffer offset before printing. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/core/io/print_pos.c |
