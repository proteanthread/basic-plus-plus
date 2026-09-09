<!--
Title:        LLIST
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/program/llist.c
Generated:    no, hand-written
Status:       current
-->

# `LLIST` Keyword Reference

## Source Header

```c
// FILENAME: llist.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, llist.h, string.c, vm.h
// NEEDS: libkernel (errors.h, security.h, security.c, vprinter.h, vprinter.c)
// Provides runtime implementation for the LLIST statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Lists program lines to the line printer device (LPT1: / virtual printer).

## 2. Syntax

```basic
LLIST [start_line] [- [end_line]]
```

## 3. Code Example

```basic
10 REM LLIST Demonstration
20 PRINT "LLIST executed successfully."
```

## 4. Error Conditions

Error 5: Illegal function call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Program Management
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | LLIST |
| Category | Program Management |
| Syntax | LLIST [start_line] [- [end_line]] |
| Description | Lists program lines to the line printer device (LPT1: / virtual printer). |
| Error Summary | Error 5: Illegal function call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/program/llist.c |
