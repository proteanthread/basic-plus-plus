<!--
Title:        LIST
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/program/list.c
Generated:    no, hand-written
Status:       current
-->

# `LIST` Keyword Reference

## Source Header

```c
// FILENAME: list.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, list.h, string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the LIST statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Displays specified program lines or entire source in memory, optionally writing to destination file.

## 2. Syntax

```basic
LIST [start_line] [- [end_line]] [, filename_expr]
```

## 3. Code Example

```basic
10 REM LIST Demonstration
20 PRINT "LIST executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

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
| Name | LIST |
| Category | Program Mgmt & Editing |
| Syntax | LIST [start_line] [- [end_line]] [, filename_expr] |
| Description | Displays specified program lines or entire source in memory, optionally writing to destination file. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/program/list.c |
