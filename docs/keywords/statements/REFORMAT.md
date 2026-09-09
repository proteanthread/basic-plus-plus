<!--
Title:        REFORMAT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/program/reformat.c
Generated:    no, hand-written
Status:       current
-->

# `REFORMAT` Keyword Reference

## Source Header

```c
// FILENAME: reformat.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (reformat_internal.h)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (lexer.h, lexer.c, reformat.h, string.c, vm.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the REFORMAT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Reformats BASIC source code with configurable indentation, unpacking, and static structural analysis.

## 2. Syntax

```basic
REFORMAT [spaces_per_indent] [target] [CHECK|STRICT|SPLIT|UPPER|LOWER|PRESERVE|SPACES|UNPACK]
```

## 3. Code Example

```basic
10 FOR I = 1 TO 5
20   PRINT "Iteration: "; I
30 NEXT I
```

## 4. Error Conditions

Error 2: Syntax Error

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
| Name | REFORMAT |
| Category | Program Mgmt & Editing |
| Syntax | REFORMAT [spaces_per_indent] [target] [CHECK\|STRICT\|SPLIT\|UPPER\|LOWER\|PRESERVE\|SPACES\|UNPACK] |
| Description | Reformats BASIC source code with configurable indentation, unpacking, and static structural analysis. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/program/reformat.c |
