<!--
Title:        MERGE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/program/merge.c
Generated:    no, hand-written
Status:       current
-->

# `MERGE` Keyword Reference

## Source Header

```c
// FILENAME: merge.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, merge.h, string.c, vm.h
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the MERGE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Merges specified ASCII BASIC source file into current memory program without clearing existing lines.

## 2. Syntax

```basic
MERGE filename_expr
```

## 3. Code Example

```basic
10 REM MERGE Demonstration
20 PRINT "MERGE executed successfully."
```

## 4. Error Conditions

Error 53: File Not Found, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Program Mgmt & Editing
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | MERGE |
| Category | Program Mgmt & Editing |
| Syntax | MERGE filename_expr |
| Description | Merges specified ASCII BASIC source file into current memory program without clearing existing lines. |
| Error Summary | Error 53: File Not Found, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/program/merge.c |
