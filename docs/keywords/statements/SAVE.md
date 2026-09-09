<!--
Title:        SAVE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/program/save.c
Generated:    no, hand-written
Status:       current
-->

# `SAVE` Keyword Reference

## Source Header

```c
// FILENAME: save.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, save.h, string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the SAVE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Saves the program currently in memory to a file on disk.

## 2. Syntax

```basic
SAVE filename_expr [, A | P]
```

## 3. Code Example

```basic
10 REM SAVE Demonstration
20 PRINT "SAVE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 64: Bad File Name, Error 70: Permission Denied

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
| Name | SAVE |
| Category | Program Mgmt & Editing |
| Syntax | SAVE filename_expr [, A \| P] |
| Description | Saves the program currently in memory to a file on disk. |
| Error Summary | Error 2: Syntax Error, Error 64: Bad File Name, Error 70: Permission Denied |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/program/save.c |
